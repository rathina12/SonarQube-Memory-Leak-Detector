package io.mlpca.sonar;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import org.sonar.api.batch.fs.FileSystem;
import org.sonar.api.batch.fs.InputFile;
import org.sonar.api.batch.sensor.Sensor;
import org.sonar.api.batch.sensor.SensorContext;
import org.sonar.api.batch.sensor.SensorDescriptor;
import org.sonar.api.batch.sensor.issue.NewExternalIssue;
import org.sonar.api.batch.sensor.issue.NewIssueLocation;
import org.sonar.api.config.Configuration;
import org.sonar.api.issue.RuleType;
import org.sonar.api.utils.log.Logger;
import org.sonar.api.utils.log.Loggers;

public final class MlpcaSensor implements Sensor {
  private static final Logger LOG = Loggers.get(MlpcaSensor.class);
  private static final String ENGINE = "mlpca";
  private final Configuration configuration;
  private final FileSystem fileSystem;

  public MlpcaSensor(Configuration configuration, FileSystem fileSystem) {
    this.configuration = configuration;
    this.fileSystem = fileSystem;
  }

  @Override
  public void describe(SensorDescriptor descriptor) {
    descriptor.name("MLPCA C/C++ Memory Lifecycle Report Importer");
  }

  @Override
  public void execute(SensorContext context) {
    String[] configured = configuration.getStringArray(MlpcaPlugin.REPORT_PATHS);
    if (configured.length == 0) configured = new String[] {"mlpca-report.json"};

    int imported = 0;
    for (String raw : configured) {
      if (raw == null || raw.isBlank()) continue;
      Path report = resolveReport(raw.trim());
      if (!Files.isRegularFile(report)) {
        LOG.info("MLPCA report not found, skipping: {}", report);
        continue;
      }
      try {
        ReportModel model = ReportReader.read(report);
        for (ReportModel.Finding finding : model.issues) {
          InputFile input = resolveInputFile(finding.file, model.projectRoot);
          if (input == null) {
            LOG.warn("MLPCA finding references a file not indexed by SonarQube: {}", finding.file);
            continue;
          }
          createIssue(context, input, finding, model.projectRoot);
          imported++;
        }
      } catch (IOException | RuntimeException ex) {
        throw new IllegalStateException("Failed to import MLPCA report " + report + ": " + ex.getMessage(), ex);
      }
    }
    LOG.info("MLPCA imported {} external issue(s)", imported);
  }

  private Path resolveReport(String raw) {
    Path p = Paths.get(raw);
    return p.isAbsolute() ? p.normalize() : fileSystem.baseDir().toPath().resolve(p).normalize();
  }

  private InputFile resolveInputFile(String reportedPath, String projectRoot) {
    Path reported = Paths.get(reportedPath).normalize();
    if (reported.isAbsolute()) {
      InputFile f = fileSystem.inputFile(fileSystem.predicates().hasAbsolutePath(reported.toString()));
      if (f != null) return f;
    }

    String relative = reported.toString().replace('\\', '/');
    InputFile f = fileSystem.inputFile(fileSystem.predicates().hasRelativePath(relative));
    if (f != null) return f;

    if (projectRoot != null && !projectRoot.isBlank()) {
      try {
        Path root = Paths.get(projectRoot).toAbsolutePath().normalize();
        Path absolute = reported.isAbsolute() ? reported.toAbsolutePath().normalize() : root.resolve(reported).normalize();
        f = fileSystem.inputFile(fileSystem.predicates().hasAbsolutePath(absolute.toString()));
        if (f != null) return f;
        if (absolute.startsWith(root)) {
          String rel = root.relativize(absolute).toString().replace('\\', '/');
          f = fileSystem.inputFile(fileSystem.predicates().hasRelativePath(rel));
        }
      } catch (RuntimeException ignored) {
      }
    }
    return f;
  }

  private void createIssue(SensorContext context, InputFile input, ReportModel.Finding f, String projectRoot) {
    int line = Math.max(1, Math.min(f.line, Math.max(1, input.lines())));
    NewExternalIssue issue = context.newExternalIssue()
        .engineId(ENGINE)
        .ruleId(f.ruleId)
        .severity(mapSeverity(f.severity))
        .type(RuleType.BUG)
        .remediationEffortMinutes(remediationMinutes(f.ruleId));

    NewIssueLocation primary = issue.newLocation()
        .on(input)
        .at(input.selectLine(line))
        .message(withConfidence(f.message, f.confidence));
    issue.at(primary);

    List<NewIssueLocation> flow = new ArrayList<>();
    for (ReportModel.Flow step : f.flow) {
      InputFile flowFile = resolveInputFile(step.file, projectRoot);
      if (flowFile == null || step.line < 1) continue;
      int flowLine = Math.min(step.line, Math.max(1, flowFile.lines()));
      flow.add(issue.newLocation().on(flowFile).at(flowFile.selectLine(flowLine)).message(step.message == null ? "MLPCA flow" : step.message));
    }
    if (!flow.isEmpty()) issue.addFlow(flow);
    issue.save();
  }

  static org.sonar.api.batch.rule.Severity mapSeverity(String severity) {
    if (severity == null) return org.sonar.api.batch.rule.Severity.MAJOR;
    return switch (severity.toUpperCase()) {
      case "INFO" -> org.sonar.api.batch.rule.Severity.INFO;
      case "MINOR" -> org.sonar.api.batch.rule.Severity.MINOR;
      case "CRITICAL" -> org.sonar.api.batch.rule.Severity.CRITICAL;
      case "BLOCKER" -> org.sonar.api.batch.rule.Severity.BLOCKER;
      default -> org.sonar.api.batch.rule.Severity.MAJOR;
    };
  }

  private static long remediationMinutes(String ruleId) {
    return switch (ruleId) {
      case "ML006", "ML007", "ML008", "ML009" -> 30L;
      case "ML001", "ML002", "ML003", "ML004" -> 20L;
      default -> 15L;
    };
  }

  private static String withConfidence(String message, double confidence) {
    if (confidence <= 0.0 || confidence >= 0.999) return message;
    return message + " (MLPCA confidence " + Math.round(confidence * 100.0) + "%)";
  }
}
