package io.mlpca.sonar;

import org.sonar.api.Plugin;
import org.sonar.api.config.PropertyDefinition;

public final class MlpcaPlugin implements Plugin {
  public static final String REPORT_PATHS = "sonar.mlpca.reportPaths";

  @Override
  public void define(Context context) {
    context.addExtension(
        PropertyDefinition.builder(REPORT_PATHS)
            .name("MLPCA report paths")
            .description("Comma-separated MLPCA JSON report paths, relative to the Sonar project base directory or absolute.")
            .category("MLPCA")
            .defaultValue("mlpca-report.json")
            .multiValues(true)
            .build());
    context.addExtension(MlpcaSensor.class);
  }
}
