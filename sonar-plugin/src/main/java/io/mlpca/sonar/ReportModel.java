package io.mlpca.sonar;

import java.util.ArrayList;
import java.util.List;

final class ReportModel {
  int schemaVersion;
  String engine;
  String projectRoot;
  Metrics metrics;
  List<Finding> issues = new ArrayList<>();

  static final class Metrics {
    long filesScanned;
    long filesFailed;
    long allocations;
    long releases;
    long branchesObserved;
    long pathStatesCreated;
    long pathStatesMerged;
    long issues;
  }

  static final class Finding {
    String ruleId;
    String severity;
    String file;
    int line;
    int column;
    String message;
    String symbol;
    String allocationKind;
    double confidence;
    List<Flow> flow = new ArrayList<>();
  }

  static final class Flow {
    String file;
    int line;
    String message;
  }
}
