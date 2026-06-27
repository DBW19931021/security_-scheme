## ADDED Requirements

### Requirement: WF-REQ-001 Chip-level P0-P6 lifecycle
The workflow MUST define a chip-level lifecycle from project initialization through
release, retrospective, and reusable-knowledge extraction.

#### Scenario: A new chip security project starts
- **WHEN** a team starts security design for a new chip
- **THEN** the workflow MUST guide the project through P0 to P6
- **AND** each phase MUST declare required artifacts and exit conditions

### Requirement: WF-REQ-002 Feature-level OpenSpec loop
Each material security capability MUST use an OpenSpec change loop for its
requirements, design, implementation tasks, evidence, and retrospective.

#### Scenario: A security feature changes behavior or trust boundaries
- **WHEN** a feature affects security behavior, interfaces, protocols, keys, boot,
  lifecycle, manufacturing, or attestation
- **THEN** the project MUST create or update an OpenSpec change
- **AND** the change MUST not start implementation before its design Gate is approved

### Requirement: WF-REQ-003 Single-source ownership
The workflow MUST assign one owner system to each class of project fact.

#### Scenario: Two documents disagree
- **WHEN** a Security Workflow document and an OpenSpec change disagree about a
  chip-level architecture decision
- **THEN** the Security Workflow decision or approved Change Request MUST win
- **AND** the OpenSpec change MUST be corrected instead of silently creating a new baseline

### Requirement: WF-REQ-004 G0-G6 Gate evaluation
The workflow MUST provide structural evaluation for Gates G0 through G6.

#### Scenario: Required evidence is missing
- **WHEN** a Gate evaluator finds a required file, field, link, or test record missing
- **THEN** it MUST return `FAIL`
- **AND** the project MUST not claim the Gate is ready

#### Scenario: Structure is complete but approval is missing
- **WHEN** all mechanically checkable conditions pass but a required human approval is absent
- **THEN** the evaluator MUST return `REVIEW`
- **AND** it MUST not create an approval record

### Requirement: WF-REQ-005 Human approval authority
Automation MUST NOT approve security architecture decisions or knowledge promotion.

#### Scenario: Promotion conditions are mechanically satisfied
- **WHEN** a knowledge candidate satisfies all structural conditions for its target level
- **THEN** the evaluator MUST return `REVIEW`
- **AND** it MUST leave the candidate approval state unchanged

### Requirement: WF-REQ-006 Verification evidence
Every feature change approaching archive readiness MUST retain concise, reproducible
verification evidence.

#### Scenario: A verification command is executed
- **WHEN** a test or validation command supports a requirement
- **THEN** the evidence MUST record the command, environment, result, covered
  requirement, unverified items, and residual risk

### Requirement: WF-REQ-007 Problem and retrospective records
Non-trivial problems and completed changes MUST leave structured learning records.

#### Scenario: Debugging reveals a reusable root cause
- **WHEN** a problem crosses the configured complexity or security-impact threshold
- **THEN** the change MUST create a problem record
- **AND** the record MUST include rejected approaches, root cause, solution,
  evidence, applicability, and a counterexample

### Requirement: WF-REQ-008 Knowledge promotion levels
The workflow MUST distinguish L0 through L5 knowledge and enforce
evidence-based, human-approved promotion.

#### Scenario: A project-specific lesson is proposed as cross-chip guidance
- **WHEN** a candidate targets L3 or higher
- **THEN** it MUST state applicability, non-applicability, evidence, and counterexamples
- **AND** chip-specific addresses, names, and paths MUST be removed from the rule

### Requirement: WF-REQ-009 Onboarding paths
The workflow MUST generate progressive reading paths for new project members.

#### Scenario: A developer joins an existing project
- **WHEN** an onboarding pack is generated
- **THEN** it MUST provide a 30-minute project overview, a half-day architecture
  path, and a one-to-two-day first-change walkthrough
- **AND** it MUST link to source documents instead of copying security design bodies

### Requirement: WF-REQ-010 NGU800 pilot isolation
The first workflow pilot MUST not alter NGU800 security implementation code.

#### Scenario: The MCTP pilot is migrated
- **WHEN** the NGU800 MCTP design becomes an OpenSpec change
- **THEN** modifications MUST remain under `components/ngu_security/docs`
- **AND** no MCTP, libmctp, libspdm, solution, or production source file may change
