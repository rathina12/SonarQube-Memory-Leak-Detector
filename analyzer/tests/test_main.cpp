#include "mlpca/Analyzer.hpp"
#include "mlpca/JsonWriter.hpp"
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

static int failures=0;
#define CHECK(cond,msg) do{ if(!(cond)){ std::cerr<<"FAIL: "<<msg<<"\n"; ++failures; } else { std::cout<<"PASS: "<<msg<<"\n"; } }while(0)

static std::set<std::string> rules(const mlpca::AnalysisResult&r){std::set<std::string>s;for(auto&i:r.issues)s.insert(i.ruleId);return s;}

int main(){
  mlpca::Analyzer a;
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p = (int*)malloc(4); }\n","leak.c");
    CHECK(rules(r).count("ML001"),"ML001 detects unreleased malloc");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); free(p); }\n","safe.c");
    CHECK(r.issues.empty(),"balanced malloc/free is clean");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(int x){ int *p=(int*)malloc(4); if(x){ return; } free(p); }\n","branch.c");
    auto rs=rules(r); CHECK(rs.count("ML004")||rs.count("ML003"),"early conditional return leak detected");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); int *q=p; free(q); }\n","alias.c");
    CHECK(r.issues.empty(),"alias release frees allocation object");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); p=(int*)malloc(8); }\n","overwrite.c");
    CHECK(rules(r).count("ML002"),"pointer overwrite leak detected");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); free(p); free(p); }\n","double.c");
    CHECK(rules(r).count("ML006"),"double free detected");
  }
  {
    auto r=a.analyzeSource("void f(){ int *p=new int; free(p); }\n","mismatch.cpp");
    CHECK(rules(r).count("ML007"),"new/free mismatch detected");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); free(p); *p=3; }\n","uaf.c");
    CHECK(rules(r).count("ML009"),"use-after-free detected");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ int *p=(int*)malloc(4); p=(int*)realloc(p,8); free(p); }\n","realloc.c");
    CHECK(rules(r).count("ML008"),"unsafe direct realloc assignment detected");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nint *make(){ int *p=(int*)malloc(4); return p; } void f(){ int *q=make(); free(q); }\n","inter.c");
    CHECK(r.issues.empty(),"returned ownership summary prevents false leak");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nvoid f(){ for(int i=0;i<10;i++){ int *p=(int*)malloc(4); } }\n","loop.c");
    CHECK(rules(r).count("ML010"),"loop allocation risk detected");
  }
  {
    auto r=a.analyzeSource("void f(){ int *p=new int; delete p; }\n","new.cpp");
    CHECK(r.issues.empty(),"new/delete is clean");
  }
  {
    auto r=a.analyzeSource("void f(){ int *p=new int[5]; delete [] p; }\n","array.cpp");
    CHECK(r.issues.empty(),"new[]/delete[] is clean");
  }
  {
    auto r=a.analyzeSource("#include <stdlib.h>\nint *make(){ return (int*)malloc(4); }\n","ret.c");
    CHECK(r.issues.empty(),"ownership returned from function is not local leak");
  }
  if(failures){ std::cerr<<failures<<" test(s) failed\n"; return 1; }
  std::cout<<"All tests passed\n"; return 0;
}
