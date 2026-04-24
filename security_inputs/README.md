# Security Inputs Workspace

Use this directory to stage the materials that feed the `ngu800-security-design` skill.

Recommended structure:

```text
security_inputs/
├── soc_arch/
├── boot_flow/
├── ip_manuals/
│   ├── ehsm/
│   ├── trng/
│   └── debug/
├── board/
│   ├── bmc/
│   └── oob_mcu/
├── host/
└── requirements/
```

Suggested placement:

| Directory | Put these documents here |
|---|---|
| `soc_arch/` | SoC block diagrams, trust boundary notes, firmware domain descriptions |
| `boot_flow/` | BootROM flow, SEC handoff, image loading notes, reset sequencing |
| `ip_manuals/ehsm/` | eHSM, key ladder, KDF, crypto service manuals |
| `ip_manuals/trng/` | TRNG interface, entropy health test, conditioning notes |
| `ip_manuals/debug/` | JTAG, lifecycle, authenticated debug, trace restrictions |
| `board/bmc/` | BMC trust model, update path, management interface notes |
| `board/oob_mcu/` | OOB-MCU role, command path, reset/power/debug influence |
| `host/` | SPDM, DOE, PCIe, PLDM, attestation or host-side update requirements |
| `requirements/` | Frozen decisions, feature lists, risk registers, security requirements |

Recommended usage:

1. Put files into the closest matching directory.
2. Register the file in `inputs_manifest.md`.
3. When a file supersedes another one, record that in the manifest instead of deleting history silently.
4. For large PDFs, add a short extracted summary note near the original file if helpful.

For each source, also record usage intent:
- what parts must be followed
- what parts are only optional reference
- what parts should be ignored
- any project-specific notes that should override default reading

Minimum useful inputs by mode:

| Mode | Minimum useful input set |
|---|---|
| Full design | `soc_arch/`, `boot_flow/`, at least one relevant `ip_manuals/` source, plus `board/` or explicit board assumptions |
| Incremental update | baseline design, changed documents, and source version/date metadata |
| Deep dive | one strong topic source plus enough architecture context to place trust boundaries correctly |
