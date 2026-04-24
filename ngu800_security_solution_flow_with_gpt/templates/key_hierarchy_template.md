# NGU800 Key Hierarchy Template

## Purpose
Use this template when defining NGU800 key hierarchy / seed derivation / usage separation.

## Required Sections

### 1. Root anchor
Clarify:
- Root Secret / UDS / root seed source
- storage location
- visibility restriction
- lifecycle influence

### 2. Branch split
At minimum discuss:
- attestation identity branch
- firmware protection branch
- optional debug/auth branch
- optional board binding branch

### 3. KDF context and labels
Prefer explicit labels such as:
- "DEVICE_ID"
- "BOOT_ALIAS"
- "SEC_ALIAS"
- "RUNTIME"
- "FW_KEK"
If project uses other labels, state them clearly.

### 4. Object table
Prefer this format:

| Object | Derived From | Visibility | Usage | Owner | Notes |
|---|---|---|---|---|---|

### 5. Device identity path
Clarify:
- whether identity key is derived on device
- whether CSR flow is used
- how cert issuance works
- whether static or dynamic alias certs are used

### 6. Firmware protection path
Clarify:
- whether FW_KEK exists
- whether CEK wrapping is used
- whether confidentiality is mandatory or optional
- how key usage is separated from attestation branch

### 7. Domestic and international mapping
Provide concise mapping of signature / hash / encryption choices.

### 8. Open issues checklist
Examples:
- raw secret ownership
- KDF implementation location
- device-to-board binding need
- ownership transfer impact
