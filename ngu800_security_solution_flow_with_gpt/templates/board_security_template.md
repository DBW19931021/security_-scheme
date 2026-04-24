# NGU800 Board-Level Security Template

## Purpose
Use this template when designing NGU800 board-level security, especially around BMC, OOB-MCU, SMBus, and sideband control paths.

## Required Sections

### 1. Board trust model
Clarify:
- whether BMC is trusted, semi-trusted, or untrusted
- whether OOB-MCU is trusted, semi-trusted, or untrusted
- whether host and board controller have separate authority domains

### 2. Attack surface inventory
Consider at minimum:
- SMBus / I2C-like sideband access
- OOB command path
- board firmware update path
- reset / power / strap / debug influence
- board-originated measurement or identity claims

### 3. BMC interaction policy
Clarify:
- what BMC can read
- what BMC can command
- whether BMC can proxy attestation
- whether BMC can trigger update / recovery / debug paths

### 4. OOB-MCU interaction policy
Clarify:
- firmware trust source
- command authority
- rate / privilege limits
- dependency on SEC policy checks

### 5. SMBus / sideband security
Clarify:
- sensitive commands
- authentication need
- filtering / gating
- secure defaults in PROD

### 6. Board binding considerations
Clarify whether NGU800 should bind to:
- board_id
- platform policy
- BMC cert / identity
- deployment SKU

### 7. Recommended mitigations
Provide specific actions such as:
- command whitelist
- lifecycle gating
- attested board proxy mode
- disabling raw debug triggers from board side in PROD

### 8. Open issues checklist
Examples:
- real board topology
- BMC ownership
- OOB update authority
- sideband isolation mechanism
