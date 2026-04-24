# NGU800 Manufacturing and Provisioning Template

## Purpose
Use this template when designing manufacturing, provisioning, closeout, and RMA security flows for NGU800.

## Required Sections

### 1. Provisioning stages
Break into stages such as:
- wafer / package stage if relevant
- board assembly stage
- initial bring-up
- security provisioning
- production closeout
- field / RMA stage

### 2. Secret and identity provisioning
Clarify:
- what is injected
- what is generated on device
- whether CSR is generated on device
- what external CA / HSM / signing service is assumed

### 3. Lifecycle transition during manufacturing
Clarify:
- initial lifecycle
- when DEV closes
- when PROD is entered
- who authorizes state transition

### 4. Key and cert object flow
Prefer a sequence-like explanation:
- seed / secret source
- derived device identity
- CSR generation
- cert issuance
- device storage / blob storage / report usage

### 5. Closeout checks
Clarify:
- debug closure
- rollback floor initialization
- active key selection
- production fuse programming
- audit record requirements

### 6. RMA flow
Clarify:
- entry to RMA
- temporary policy relaxation
- data handling
- exit or decommission rule

### 7. Open issues checklist
Examples:
- manufacturing ownership split
- HSM integration
- cert chain storage location
- field replacement / board swap implications
