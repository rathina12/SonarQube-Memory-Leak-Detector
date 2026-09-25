# SonarQube Plugin

This scanner-side plugin imports `mlpca-report.json` findings as **external issues** using SonarQube's `SensorContext.newExternalIssue()` API. It does not register a competing C/C++ language, so it can coexist with existing language analyzers.

## Build

```bash
mvn clean package
```

## Install

Copy `target/sonar-mlpca-plugin-1.0.0.jar` to `$SONARQUBE_HOME/extensions/plugins/`, restart SonarQube, generate the MLPCA report before the scanner runs, then configure:

```properties
sonar.mlpca.reportPaths=mlpca-report.json
```

Multiple paths are supported through Sonar's multi-value property handling.
