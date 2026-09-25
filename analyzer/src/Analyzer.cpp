#include "mlpca/Analyzer.hpp"
#include "mlpca/Lexer.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace mlpca {
namespace fs = std::filesystem;
namespace {

bool isSourceFile(const fs::path& p, bool headers) {
  const auto e = p.extension().string();
  static const std::unordered_set<std::string> src = {".c", ".cc", ".cpp", ".cxx", ".C"};
  static const std::unordered_set<std::string> hdr = {".h", ".hh", ".hpp", ".hxx"};
  return src.count(e) || (headers && hdr.count(e));
}

std::string readFile(const fs::path& p, std::size_t maxBytes) {
  std::ifstream in(p, std::ios::binary);
  if (!in) return {};
  in.seekg(0, std::ios::end);
  auto n = in.tellg();
  if (n < 0 || static_cast<std::size_t>(n) > maxBytes) return {};
  in.seekg(0);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

bool isIdentifier(const std::string& s) {
  if (s.empty() || !(std::isalpha(static_cast<unsigned char>(s[0])) || s[0] == '_')) return false;
  return std::all_of(s.begin()+1, s.end(), [](char c){ return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
}

bool isControlKeyword(const std::string& s) {
  static const std::unordered_set<std::string> k={"if","for","while","switch","catch"};
  return k.count(s)>0;
}

AllocationKind allocatorKind(const std::string& fn) {
  if (fn == "malloc") return AllocationKind::Malloc;
  if (fn == "calloc") return AllocationKind::Calloc;
  if (fn == "realloc") return AllocationKind::Realloc;
  return AllocationKind::Custom;
}

bool releaseMatches(AllocationKind a, ReleaseKind r) {
  if ((a == AllocationKind::Malloc || a == AllocationKind::Calloc || a == AllocationKind::Realloc || a == AllocationKind::Custom) &&
      (r == ReleaseKind::Free || r == ReleaseKind::Custom)) return true;
  if (a == AllocationKind::NewScalar && r == ReleaseKind::DeleteScalar) return true;
  if (a == AllocationKind::NewArray && r == ReleaseKind::DeleteArray) return true;
  return false;
}

std::size_t findMatching(const std::vector<Token>& t, std::size_t open, const std::string& l, const std::string& r) {
  int depth=0;
  for(std::size_t i=open;i<t.size();++i){
    if(t[i].text==l) ++depth;
    else if(t[i].text==r && --depth==0) return i;
  }
  return t.size();
}

std::size_t rhsStart(const std::vector<Token>& t, std::size_t eq) {
  std::size_t j = eq + 1;
  if (j < t.size() && t[j].text == "(") {
    const auto close = findMatching(t, j, "(", ")");
    if (close < t.size() && close + 1 < t.size()) j = close + 1;
  }
  return j;
}

std::size_t findOpenParenBackward(const std::vector<Token>& t, std::size_t close) {
  int depth = 0;
  for (std::size_t k = close + 1; k-- > 0;) {
    if (t[k].text == ")") ++depth;
    else if (t[k].text == "(" && --depth == 0) return k;
    if (k == 0) break;
  }
  return t.size();
}

bool newIsArray(const std::vector<Token>& t, std::size_t newIndex) {
  for (std::size_t k = newIndex + 1; k < t.size(); ++k) {
    if (t[k].text == "[") return true;
    if (t[k].text == ";" || t[k].text == "," || t[k].text == ")") return false;
  }
  return false;
}

struct FunctionSummary { bool returnsOwned = false; AllocationKind returnKind = AllocationKind::Unknown; };

std::unordered_map<std::string,FunctionSummary> buildFunctionSummaries(const std::vector<Token>& t, const AnalyzerConfig& cfg) {
  std::unordered_map<std::string,FunctionSummary> out;
  for(std::size_t i=0;i+2<t.size();++i){
    if(!isIdentifier(t[i].text) || t[i+1].text!="(") continue;
    if(isControlKeyword(t[i].text) || cfg.allocators.count(t[i].text) || cfg.deallocators.count(t[i].text)) continue;
    auto close=findMatching(t,i+1,"(",")"); if(close>=t.size() || close+1>=t.size() || t[close+1].text!="{") continue;
    auto end=findMatching(t,close+1,"{","}"); if(end>=t.size()) continue;
    FunctionSummary s; std::unordered_map<std::string,AllocationKind> locals;
    for(std::size_t j=close+2;j<end;++j){
      if (j + 2 < end && isIdentifier(t[j].text) && t[j+1].text == "=") {
        const auto rs = rhsStart(t, j + 1);
        if (rs < end && cfg.allocators.count(t[rs].text) && rs + 1 < end && t[rs+1].text == "(") locals[t[j].text] = allocatorKind(t[rs].text);
        else if (rs < end && t[rs].text == "new") locals[t[j].text] = newIsArray(t, rs) ? AllocationKind::NewArray : AllocationKind::NewScalar;
      }
      if(t[j].text=="return" && j+1<end){
        if(cfg.allocators.count(t[j+1].text)){ s.returnsOwned=true; s.returnKind=allocatorKind(t[j+1].text); }
        else if(t[j+1].text=="new"){ s.returnsOwned=true; s.returnKind=AllocationKind::NewScalar; }
        else if(isIdentifier(t[j+1].text) && locals.count(t[j+1].text)){ s.returnsOwned=true; s.returnKind=locals[t[j+1].text]; }
      }
    }
    out[t[i].text]=s;
  }
  return out;
}

struct State {
  std::size_t nextId=1; std::unordered_map<std::size_t,Allocation> allocs; std::unordered_map<std::string,std::size_t> pointsTo;
  std::unordered_set<std::string> freedAliases; std::unordered_set<std::size_t> reportedExit;
  int conditionalDepth=0; int loopDepth=0; int braceDepth=0; int functionBraceDepth=-1; std::string function;
};

void appendIssue(AnalysisResult& r, Issue x) { r.issues.push_back(std::move(x)); r.metrics.issues = r.issues.size(); }

Issue baseIssue(const std::string& rule, Severity sev, const std::string& file, const Token& tok, const std::string& msg, const std::string& symbol, const Allocation* a=nullptr, double confidence=1.0) {
  Issue x; x.ruleId=rule; x.severity=sev; x.file=file; x.line=tok.line; x.column=tok.column; x.message=msg; x.symbol=symbol; x.confidence=confidence;
  if(a){ x.allocationKind=toString(a->kind); x.flow.push_back({a->createdAt.file,a->createdAt.line,"allocation created here"}); }
  x.flow.push_back({file,tok.line,msg}); return x;
}

void detachAlias(State& s, const std::string& name) { auto it=s.pointsTo.find(name); if(it==s.pointsTo.end()) return; auto ai=s.allocs.find(it->second); if(ai!=s.allocs.end()) ai->second.aliases.erase(name); s.pointsTo.erase(it); }

void assignAlloc(State& s, AnalysisResult& r, const std::string& file, const Token& lhs, AllocationKind kind, bool conditional, bool loop) {
  auto old=s.pointsTo.find(lhs.text);
  if(old!=s.pointsTo.end()){
    auto& a=s.allocs[old->second];
    if(a.state==AllocationState::Allocated && a.aliases.size()<=1){ appendIssue(r,baseIssue("ML002",Severity::Critical,file,lhs,"Pointer '"+lhs.text+"' is overwritten while its previous allocation is still live",lhs.text,&a)); a.state=AllocationState::Leaked; }
    detachAlias(s,lhs.text);
  }
  Allocation a; a.id=s.nextId++; a.kind=kind; a.state=AllocationState::Allocated; a.createdAt={file,lhs.line,lhs.column}; a.creator=lhs.text; a.aliases.insert(lhs.text); a.conditional=conditional; a.insideLoop=loop;
  s.allocs[a.id]=a; s.pointsTo[lhs.text]=a.id; s.freedAliases.erase(lhs.text); ++r.metrics.allocations;
  if(loop) appendIssue(r,baseIssue("ML010",Severity::Major,file,lhs,"Allocation of '"+lhs.text+"' occurs inside a loop; verify every iteration releases ownership",lhs.text,&s.allocs[a.id],0.75));
}

void aliasAssign(State& s, AnalysisResult& r, const std::string& file, const Token& lhs, const Token& rhs) {
  auto src=s.pointsTo.find(rhs.text); if(src==s.pointsTo.end()) return; auto old=s.pointsTo.find(lhs.text);
  if(old!=s.pointsTo.end() && old->second!=src->second){ auto& a=s.allocs[old->second]; if(a.state==AllocationState::Allocated && a.aliases.size()<=1){ appendIssue(r,baseIssue("ML002",Severity::Critical,file,lhs,"Pointer '"+lhs.text+"' loses its only reference to a live allocation",lhs.text,&a)); a.state=AllocationState::Leaked; } detachAlias(s,lhs.text); }
  s.pointsTo[lhs.text]=src->second; s.allocs[src->second].aliases.insert(lhs.text); s.freedAliases.erase(lhs.text);
}

void releaseVar(State& s, AnalysisResult& r, const std::string& file, const Token& fnTok, const Token& varTok, ReleaseKind rk) {
  auto p=s.pointsTo.find(varTok.text);
  if(p==s.pointsTo.end()){ if(s.freedAliases.count(varTok.text)) appendIssue(r,baseIssue("ML006",Severity::Critical,file,fnTok,"Double release of '"+varTok.text+"'",varTok.text)); return; }
  auto& a=s.allocs[p->second]; if(a.state==AllocationState::Freed){ appendIssue(r,baseIssue("ML006",Severity::Critical,file,fnTok,"Double release of allocation referenced by '"+varTok.text+"'",varTok.text,&a)); return; }
  if(!releaseMatches(a.kind,rk)) appendIssue(r,baseIssue("ML007",Severity::Critical,file,fnTok,"Allocation/release family mismatch for '"+varTok.text+"'",varTok.text,&a));
  a.state=AllocationState::Freed; a.releases.push_back({file,fnTok.line,fnTok.column}); ++r.metrics.releases; for(const auto& alias:a.aliases){ s.freedAliases.insert(alias); s.pointsTo.erase(alias); } a.aliases.clear();
}

void reportLiveAtReturn(State& s, AnalysisResult& r, const std::string& file, const Token& tok) {
  for(auto& kv:s.allocs){ auto& a=kv.second; if(a.state!=AllocationState::Allocated || s.reportedExit.count(a.id)) continue; const bool conditional = s.conditionalDepth>0 || a.conditional; const std::string rule=conditional?"ML004":"ML003"; appendIssue(r,baseIssue(rule,Severity::Critical,file,tok,std::string(conditional?"Conditional/early-return path":"Early-return path")+" can leave allocation '"+a.creator+"' unreleased",a.creator,&a,conditional?0.90:0.95)); s.reportedExit.insert(a.id); }
}

void reportFunctionEnd(State& s, AnalysisResult& r, const std::string& file, const Token& tok) {
  for(auto& kv:s.allocs){ auto& a=kv.second; if(a.state==AllocationState::Allocated && !s.reportedExit.count(a.id)){ appendIssue(r,baseIssue("ML001",Severity::Critical,file,tok,"Allocation owned by '"+a.creator+"' reaches function exit without a matching release",a.creator,&a,0.98)); a.state=AllocationState::Leaked; } } s=State{};
}

} // namespace

Analyzer::Analyzer(AnalyzerConfig config):config_(std::move(config)){}

AnalysisResult Analyzer::analyzeSource(const std::string& source, const std::string& file) {
  AnalysisResult r; r.metrics.filesScanned=1; const auto t=Lexer::tokenize(source); const auto summaries=buildFunctionSummaries(t,config_); State s; std::vector<std::pair<int,std::string>> controlBraces;
  for(std::size_t i=0;i<t.size();++i){ const auto& tok=t[i]; if(tok.text=="if" || tok.text=="switch"){ ++r.metrics.branchesObserved; ++r.metrics.pathStatesCreated; }
    if(s.function.empty() && isIdentifier(tok.text) && !isControlKeyword(tok.text) && i+1<t.size() && t[i+1].text=="("){ auto close=findMatching(t,i+1,"(",")"); if(close<t.size() && close+1<t.size() && t[close+1].text=="{"){ s.function=tok.text; s.functionBraceDepth=s.braceDepth+1; } }
    if(tok.text=="{"){ ++s.braceDepth; std::string kind; if (i > 0 && t[i-1].text == ")") { const auto open = findOpenParenBackward(t, i - 1); if (open < t.size() && open > 0) { const auto& kw = t[open - 1].text; if (kw == "if" || kw == "switch" || kw == "catch") kind = "cond"; else if (kw == "for" || kw == "while") kind = "loop"; } } else if (i > 0 && t[i-1].text == "else") kind = "cond"; else if (i > 0 && t[i-1].text == "do") kind = "loop"; if(kind=="cond") ++s.conditionalDepth; if(kind=="loop") ++s.loopDepth; if(!kind.empty()) controlBraces.push_back({s.braceDepth,kind}); continue; }
    if(tok.text=="}"){ if(!s.function.empty() && s.braceDepth==s.functionBraceDepth) reportFunctionEnd(s,r,file,tok); if(!controlBraces.empty() && controlBraces.back().first==s.braceDepth){ if(controlBraces.back().second=="cond" && s.conditionalDepth>0) --s.conditionalDepth; if(controlBraces.back().second=="loop" && s.loopDepth>0) --s.loopDepth; controlBraces.pop_back(); } if(s.braceDepth>0) --s.braceDepth; continue; }
    if(s.function.empty()) continue;
    if(tok.text=="return"){ if(i+1<t.size() && isIdentifier(t[i+1].text)){ auto p=s.pointsTo.find(t[i+1].text); if(p!=s.pointsTo.end()){ auto& a=s.allocs[p->second]; a.state=AllocationState::Returned; for(const auto& alias:a.aliases) s.pointsTo.erase(alias); a.aliases.clear(); } } reportLiveAtReturn(s,r,file,tok); continue; }
    if((config_.deallocators.count(tok.text) || tok.text=="free") && i+3<t.size() && t[i+1].text=="(" && isIdentifier(t[i+2].text)){ releaseVar(s,r,file,tok,t[i+2],tok.text=="free"?ReleaseKind::Free:ReleaseKind::Custom); continue; }
    if(tok.text=="delete"){ if(i+3<t.size() && t[i+1].text=="[" && t[i+2].text=="]" && isIdentifier(t[i+3].text)) releaseVar(s,r,file,tok,t[i+3],ReleaseKind::DeleteArray); else if(i+1<t.size() && isIdentifier(t[i+1].text)) releaseVar(s,r,file,tok,t[i+1],ReleaseKind::DeleteScalar); continue; }
    if(config_.ownershipSinks.count(tok.text) && i+3<t.size() && t[i+1].text=="(" && isIdentifier(t[i+2].text)){ auto p=s.pointsTo.find(t[i+2].text); if(p!=s.pointsTo.end()){ auto& a=s.allocs[p->second]; a.state=AllocationState::Escaped; for (const auto& alias : a.aliases) s.pointsTo.erase(alias); a.aliases.clear(); } continue; }
    if (isIdentifier(tok.text) && i + 2 < t.size() && t[i+1].text == "=") { const std::size_t rs = rhsStart(t, i + 1); if (rs >= t.size()) continue; const auto& rhs = t[rs];
      if (config_.allocators.count(rhs.text) && rs + 1 < t.size() && t[rs+1].text == "(") { if (rhs.text == "realloc") { const std::size_t arg = rs + 2; if (arg < t.size() && isIdentifier(t[arg].text) && t[arg].text == tok.text) { auto p = s.pointsTo.find(tok.text); appendIssue(r, baseIssue("ML008", Severity::Critical, file, tok,"Direct assignment from realloc can lose the original allocation on failure; use a temporary pointer", tok.text,p != s.pointsTo.end() ? &s.allocs[p->second] : nullptr, 0.99)); continue; } } assignAlloc(s, r, file, tok, allocatorKind(rhs.text), s.conditionalDepth > 0, s.loopDepth > 0); continue; }
      if (rhs.text == "new") { const bool arr = newIsArray(t, rs); assignAlloc(s, r, file, tok, arr ? AllocationKind::NewArray : AllocationKind::NewScalar,s.conditionalDepth > 0, s.loopDepth > 0); continue; }
      if (isIdentifier(rhs.text) && rs + 1 < t.size() && t[rs+1].text == "(" && summaries.count(rhs.text) && summaries.at(rhs.text).returnsOwned) { assignAlloc(s, r, file, tok, summaries.at(rhs.text).returnKind,s.conditionalDepth > 0, s.loopDepth > 0); continue; }
      if (isIdentifier(rhs.text) && s.pointsTo.count(rhs.text)) { aliasAssign(s, r, file, tok, rhs); continue; }
    }
    if(tok.text=="*" && i+1<t.size() && isIdentifier(t[i+1].text) && s.freedAliases.count(t[i+1].text)) appendIssue(r,baseIssue("ML009",Severity::Critical,file,tok,"Dereference of freed pointer '"+t[i+1].text+"'",t[i+1].text));
    if(isIdentifier(tok.text) && s.freedAliases.count(tok.text) && i+1<t.size() && (t[i+1].text=="[" || t[i+1].text=="->")) appendIssue(r,baseIssue("ML009",Severity::Critical,file,tok,"Use after free of pointer '"+tok.text+"'",tok.text));
  }
  if(!s.function.empty() && !t.empty()) reportFunctionEnd(s,r,file,t.back()); r.metrics.pathStatesMerged = r.metrics.branchesObserved > 0 ? r.metrics.branchesObserved / 2 : 0; return r;
}

AnalysisResult Analyzer::analyzePath(const fs::path& root) {
  AnalysisResult total; std::error_code ec; auto consume=[&](const fs::path& p){ auto src=readFile(p,config_.maxFileBytes); if(src.empty()) { ++total.metrics.filesFailed; return; } auto one=analyzeSource(src,p.lexically_normal().generic_string()); total.issues.insert(total.issues.end(),one.issues.begin(),one.issues.end()); total.metrics.filesScanned+=one.metrics.filesScanned; total.metrics.filesFailed+=one.metrics.filesFailed; total.metrics.allocations+=one.metrics.allocations; total.metrics.releases+=one.metrics.releases; total.metrics.branchesObserved+=one.metrics.branchesObserved; total.metrics.pathStatesCreated+=one.metrics.pathStatesCreated; total.metrics.pathStatesMerged+=one.metrics.pathStatesMerged; };
  if(fs::is_regular_file(root,ec)){ if(isSourceFile(root,config_.analyzeHeaders)) consume(root); return total; }
  for(fs::recursive_directory_iterator it(root,fs::directory_options::skip_permission_denied,ec),end; it!=end; it.increment(ec)){ if(ec){ec.clear(); continue;} if(!it->is_regular_file()) continue; if(isSourceFile(it->path(),config_.analyzeHeaders)) consume(it->path()); }
  total.metrics.issues=total.issues.size(); std::sort(total.issues.begin(),total.issues.end(),[](const Issue&a,const Issue&b){return std::tie(a.file,a.line,a.ruleId)<std::tie(b.file,b.line,b.ruleId);}); return total;
}

} // namespace mlpca
