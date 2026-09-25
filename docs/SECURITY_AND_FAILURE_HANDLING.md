# Failure Handling and Defensive Design

- Individual unreadable/oversized files increment `filesFailed`; they do not crash a whole repository scan.
- Report schema and engine identifiers are validated before Sonar import.
- Sonar finding paths are resolved only against indexed files; unknown paths are warned and skipped.
- Finding line numbers are clamped to indexed file line ranges before creating a Sonar location.
- The analyzer never edits source code automatically.
- Ownership transfer can be modelled explicitly to reduce dangerous auto-fix assumptions.
- Analyzer and Sonar plugin are decoupled through JSON, making plugin API upgrades isolated.
- Source files larger than the configured limit are skipped to avoid accidental memory exhaustion.
