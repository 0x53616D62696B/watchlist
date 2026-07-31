# APP-041: Storage accepts values without a validation contract

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Storage/API
- **Location:** [`src/Utils/Storage/DatabaseItem.hpp`](../../../../src/Utils/Storage/DatabaseItem.hpp), lines 6-16; [`src/Utils/Storage/IDatabase.hpp`](../../../../src/Utils/Storage/IDatabase.hpp), lines 12-53
- **Dependencies:** None
- **Status:** Resolved

## Observation

The storage model accepts arbitrary IDs, names, and IP-address strings, including empty values, without documenting whether validation belongs to the domain layer or each backend.

## Reasoning and impact

Backends can diverge as new implementations arrive, and invalid identifiers or addresses can become durable data. Database constraints currently enforce only non-null and uniqueness, not domain meaning.

## Recommended improvement

Define invariants in a domain value type or documented service boundary before persistence. Keep backend constraints as defense in depth and return structured validation errors distinct from storage failures.

## Acceptance criteria

- Ownership of validation is explicit.
- All backends accept/reject the same domain values.
- Invalid input cannot partially mutate storage.

## Suggested tests

Create a shared backend contract suite covering empty/oversized IDs and names, address formats selected by product policy, port boundaries, and duplicate IDs.

## Resolution

Storage now owns one documented validation contract and reports `DatabaseValidationError` with a structured `DatabaseField`. IDs accept 1-128 valid UTF-8 bytes, names 1-256, and addresses valid IPv4/IPv6 literals or ASCII DNS hostnames up to 253 bytes. The API's `uint16_t` and `bool` types define the complete port and alive domains. Add, upsert, and replacement validate before mutation, including duplicate replacement IDs.

## Validation

Contract tests cover empty, oversized, malformed UTF-8, invalid address/hostname, valid Unicode, all supported address forms, duplicate IDs, and unchanged contents after every rejected mutation.
