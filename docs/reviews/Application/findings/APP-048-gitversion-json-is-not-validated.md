# APP-048: Regex parsing can generate an invalid version header

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Build/versioning
- **Location:** [`cmake/GitVersionConfig.cmake`](../../../../cmake/GitVersionConfig.cmake), lines 50-122
- **Dependencies:** APP-019, APP-020

## Observation

GitVersion JSON is parsed with independent regex matches. Required fields are not validated after each match, the regexes do not validate JSON structure, and string content is written into C/C++ string literals without escaping.

## Reasoning and impact

Schema/output changes or unusual branch/tag characters can create empty numeric macros, select text outside the intended field structure, or produce syntactically invalid generated C++. Configure may report success and fail later during compilation.

## Recommended improvement

Use CMake's JSON query support for required fields, validate types/ranges, and escape generated string literals. Fail at configure with the missing/invalid field named.

## Acceptance criteria

- Missing or wrong-typed required fields stop configuration clearly.
- Quotes, backslashes, and non-ASCII metadata generate valid C++.
- Every parse result comes from a validated JSON field of the expected type.

## Suggested tests

Feed representative valid JSON plus missing, null, reordered, escaped, and malformed field fixtures into the parser.
