package io.mlpca.sonar;

import com.google.gson.Gson;
import com.google.gson.JsonParseException;
import java.io.IOException;
import java.io.Reader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

final class ReportReader {
  private static final Gson GSON = new Gson();

  private ReportReader() {}

  static ReportModel read(Path path) throws IOException {
    try (Reader r = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
      ReportModel model = GSON.fromJson(r, ReportModel.class);
      validate(model, path);
      return model;
    } catch (JsonParseException ex) {
      throw new IOException("Invalid MLPCA JSON report: " + path + ": " + ex.getMessage(), ex);
    }
  }

  static void validate(ReportModel model, Path path) throws IOException {
    if (model == null) throw new IOException("Empty MLPCA report: " + path);
    if (model.schemaVersion != 1) throw new IOException("Unsupported MLPCA schemaVersion " + model.schemaVersion + " in " + path);
    if (!"mlpca".equals(model.engine)) throw new IOException("Unexpected MLPCA engine id in " + path);
    if (model.issues == null) model.issues = new java.util.ArrayList<>();
    for (ReportModel.Finding f : model.issues) {
      if (f == null || blank(f.ruleId) || blank(f.file) || blank(f.message) || f.line < 1) {
        throw new IOException("Malformed MLPCA finding in " + path);
      }
      if (f.flow == null) f.flow = new java.util.ArrayList<>();
    }
  }

  private static boolean blank(String s) { return s == null || s.isBlank(); }
}
