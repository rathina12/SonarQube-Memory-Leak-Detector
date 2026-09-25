#include "mlpca/Analyzer.hpp"
#include "mlpca/Config.hpp"
#include "mlpca/JsonWriter.hpp"
#include <filesystem>
#include <iostream>
#include <string>

namespace {
void usage(){
  std::cerr << "Usage: mlpca-analyzer <path> [--output report.json] [--config file] [--headers] [--fail-on critical|major|none]\n";
}
int rank(mlpca::Severity s){switch(s){case mlpca::Severity::Critical:return 3;case mlpca::Severity::Major:return 2;case mlpca::Severity::Minor:return 1;case mlpca::Severity::Info:return 0;}return 0;}
}
int main(int argc,char**argv){
  if(argc<2){usage();return 2;}
  std::string input=argv[1], output="mlpca-report.json", configPath, failOn="none"; bool headers=false;
  for(int i=2;i<argc;++i){std::string a=argv[i];
    if(a=="--output"&&i+1<argc)output=argv[++i]; else if(a=="--config"&&i+1<argc)configPath=argv[++i];
    else if(a=="--headers")headers=true; else if(a=="--fail-on"&&i+1<argc)failOn=argv[++i]; else if(a=="--help"){usage();return 0;} else {std::cerr<<"Unknown option: "<<a<<"\n";return 2;}
  }
  auto cfg=mlpca::loadConfig(configPath); cfg.analyzeHeaders=headers;
  if(!std::filesystem::exists(input)){std::cerr<<"Input path does not exist: "<<input<<"\n";return 2;}
  mlpca::Analyzer analyzer(cfg); auto r=analyzer.analyzePath(input);
  if(!mlpca::writeJsonReport(r,std::filesystem::absolute(input).generic_string(),output)){std::cerr<<"Failed to write report: "<<output<<"\n";return 3;}
  std::cout<<"MLPCA: scanned="<<r.metrics.filesScanned<<" failed="<<r.metrics.filesFailed<<" allocations="<<r.metrics.allocations<<" releases="<<r.metrics.releases<<" issues="<<r.issues.size()<<"\n";
  for(const auto& x:r.issues) std::cout<<x.file<<":"<<x.line<<" ["<<x.ruleId<<"/"<<mlpca::toString(x.severity)<<"] "<<x.message<<"\n";
  if(failOn!="none"){
    int threshold=failOn=="critical"?3:2; for(const auto&x:r.issues) if(rank(x.severity)>=threshold) return 10;
  }
  return 0;
}
