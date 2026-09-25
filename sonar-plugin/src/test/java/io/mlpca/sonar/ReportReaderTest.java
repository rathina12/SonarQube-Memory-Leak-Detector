package io.mlpca.sonar;

import static org.junit.jupiter.api.Assertions.*;
import java.nio.file.Files;
import java.nio.file.Path;
import org.junit.jupiter.api.Test;

class ReportReaderTest {
  @Test
  void readsValidReport() throws Exception {
    Path p = Files.createTempFile("mlpca", ".json");
    Files.writeString(p, "{\"schemaVersion\":1,\"engine\":\"mlpca\",\"projectRoot\":\".\",\"issues\":[{\"ruleId\":\"ML001\",\"severity\":\"CRITICAL\",\"file\":\"a.c\",\"line\":1,\"column\":1,\"message\":\"leak\",\"confidence\":1.0,\"flow\":[]}]}");
    ReportModel r = ReportReader.read(p);
    assertEquals(1, r.issues.size());
    assertEquals("ML001", r.issues.get(0).ruleId);
  }

  @Test
  void rejectsWrongSchema() throws Exception {
    Path p = Files.createTempFile("mlpca", ".json");
    Files.writeString(p, "{\"schemaVersion\":99,\"engine\":\"mlpca\",\"issues\":[]}");
    assertThrows(java.io.IOException.class, () -> ReportReader.read(p));
  }
}
