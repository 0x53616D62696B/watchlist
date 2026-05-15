# Software Licensing Basics

Software licenses describe what you are allowed to do with someone else's code.
They matter when a project uses third-party libraries, especially if the application may be distributed to other people or used commercially.

This document is a practical overview, not legal advice.
For important commercial releases, check the actual license text and ask a qualified lawyer if needed.

## Common Terms

### Source Code

Source code is the human-readable code used to build the program.
For this project, that means files such as `.cpp`, `.hpp`, `CMakeLists.txt`, and other project files.

### Binary Or Executable

A binary is the compiled program that users run.
On Windows, this is often an `.exe` or `.dll`.

### Distribution

Distribution means giving the software to someone else.

Examples:

- Sending a compiled `.exe` to a user.
- Publishing a release download.
- Selling the application.
- Shipping the application to a customer.
- Publishing the source code online.

Licenses often matter most when software is distributed.
Using code only on your own machine usually creates fewer obligations, but the exact rules depend on the license.

### Commercial Use

Commercial use means using software in a business context or in a product that makes money.

Examples:

- Selling an application.
- Using an application inside a company.
- Shipping software to paying customers.
- Including a library in a paid product.

Commercial use does not automatically mean closed-source.
An app can be commercial and open-source, commercial and closed-source, free and open-source, or free and closed-source.

### Closed-Source App

A closed-source app is an app where users receive the compiled program but do not receive the source code.

Example:

```text
You ship Watchlist.exe.
Users can run it.
Users cannot see or modify the C++ source code.
```

Closed-source does not mean the app is secret or unpublished.
It only means the source code is not provided to users.

### Open-Source App

An open-source app provides source code under a license that allows users to read, modify, and redistribute it.

Example:

```text
You publish the project source code on GitHub under the MIT license.
Users can inspect, build, and modify the code.
```

## Main License Families

Most software licenses fall into three broad groups.

| License family | Examples | Practical meaning |
| --- | --- | --- |
| Permissive | MIT, BSD, Boost, Apache | Easy to use in closed-source and commercial software. Usually requires preserving copyright and license notices. |
| Weak copyleft | LGPL, MPL | Often usable in closed-source apps, but changes to the licensed library itself usually need to remain under the same license. |
| Strong copyleft | GPL, AGPL | If distributed as part of an app, the app may need to be released under a compatible open-source license. |

## Licenses Mentioned For Database Libraries

| License | Commercial use | Closed-source app | Main obligation |
| --- | --- | --- | --- |
| Public Domain | Yes | Yes | Usually none. SQLite core is public domain. |
| MIT | Yes | Yes | Keep the copyright and license notice. |
| BSD | Yes | Yes | Keep the copyright and license notice. Some variants restrict using project or author names for endorsement. |
| Boost Software License | Yes | Yes | Very permissive. Keep license text with source distributions. Binary distribution is simple. |
| LGPL 2.1 | Yes | Usually yes | If you modify the LGPL library, publish those library changes. Users should be able to replace or relink the LGPL library. |
| GPL 2/3 | Yes | Usually no for proprietary distributed apps | If you distribute a combined or derived program, the program generally must be GPL-compatible. |
| AGPL | Yes | Usually no for proprietary server apps | Similar to GPL, but also applies when users interact with the software over a network. |

## Permissive Licenses

Permissive licenses are usually the easiest licenses for application development.

Common examples:

- Public Domain
- MIT
- BSD
- Boost Software License
- Apache License

These licenses generally allow:

- Personal use.
- Commercial use.
- Closed-source use.
- Modification.
- Redistribution.

The usual requirement is to keep copyright and license notices.

For a closed-source app, permissive licenses are usually low friction because they do not require your own application source code to be published.

## Public Domain

Public domain code is not controlled by normal copyright restrictions in the same way licensed code is.
The SQLite core is public domain.

In practice, this is one of the easiest options for commercial and closed-source applications.

Some companies still buy a warranty or commercial document for SQLite because their legal department wants written proof or indemnity.
That is a business/legal policy decision, not usually a technical requirement.

## MIT And BSD

MIT and BSD are permissive licenses.
They are very common in C and C++ libraries.

They usually allow using the library in a closed-source commercial app as long as the copyright and license notices are preserved.

For example, if a project uses an MIT-licensed library, the app can still be closed-source.
The MIT library's license notice should remain in the source distribution, documentation, installer, or third-party notices file as appropriate.

## Boost Software License

The Boost Software License is also permissive.
It is commonly used by C++ libraries.

It is friendly to commercial and closed-source applications.
One practical difference from MIT/BSD is that binary-only distribution is especially simple because the license does not require the license text to accompany object code or executables.

Still, keeping third-party notices is a good habit.

## LGPL

LGPL means GNU Lesser General Public License.
It is a weak copyleft license.

LGPL is designed so that applications can use an LGPL library without forcing the whole application to become GPL or LGPL.
This can work for closed-source commercial apps, but there are extra obligations.

Typical LGPL obligations include:

- If you modify the LGPL library itself, publish those modifications under the LGPL.
- Keep license notices.
- Allow users to replace or relink the LGPL library with a modified version.

Dynamic linking usually makes LGPL compliance easier.
Static linking can be more complicated because users may need a way to relink the application with a modified version of the LGPL library.

For proprietary software, LGPL is often acceptable but should be handled carefully.

## GPL

GPL means GNU General Public License.
It is a strong copyleft license.

GPL allows commercial use.
The important issue is not money.
The important issue is distribution and source-code obligations.

If you distribute an application that is a combined or derived work based on GPL code, you generally need to distribute the application under the GPL or a GPL-compatible license.
That usually means providing the source code to recipients.

This is why GPL libraries are often avoided in closed-source applications.

## AGPL

AGPL means GNU Affero General Public License.
It is similar to GPL, but has an extra network-use condition.

With GPL, obligations are usually triggered by distribution.
With AGPL, obligations can also be triggered when users interact with the software over a network.

This matters for server software and web services.
If a proprietary server app uses AGPL code, users of the network service may need to receive access to the source code.

For closed-source products, AGPL is usually the most restrictive common open-source license.

## GPL-Compatible

GPL-compatible means a license can legally be combined with GPL code and distributed under the GPL's terms.

For example:

- MIT is GPL-compatible.
- BSD is usually GPL-compatible.
- Boost is GPL-compatible.
- Apache 2.0 is GPLv3-compatible, but not GPLv2-only compatible.

A license can be permissive and GPL-compatible.
That does not mean your project automatically becomes GPL.
It means that if someone combines that code with GPL code, the licenses do not conflict.

GPL compatibility matters when a project includes GPL components.
If one dependency is GPL, every other dependency must allow being distributed together under GPL-compatible terms.

## Linking And Why It Matters

C++ libraries are often used by linking.

There are two common forms:

- Dynamic linking: the app uses a separate `.dll` or shared library at runtime.
- Static linking: the library code is copied into the final executable during build.

Permissive licenses usually allow both without much trouble.

LGPL cares more about linking because users should be able to replace or relink the LGPL library.
Dynamic linking is often easier for LGPL compliance.

GPL linking is more restrictive.
If a closed-source app links to a GPL library and is distributed, that can create a requirement to release the app under the GPL.

## Practical Database Library Examples

| Library | License | Practical risk for closed-source commercial app |
| --- | --- | --- |
| SQLite | Public Domain | Very low |
| SQLiteCpp | MIT | Very low |
| SOCI | Boost Software License | Very low |
| libpqxx | BSD | Very low |
| MariaDB Connector/C++ | LGPL 2.1 | Usually acceptable, but pay attention to linking and modifications |
| MySQL Connector/C++ | GPL or commercial license | Risky for closed-source unless using a commercial license or complying with GPL |

For a small desktop app that wants low licensing friction, `SQLite` with an MIT-licensed C++ wrapper such as `SQLiteCpp` is a clean choice.

## Practical Rules

- Prefer permissive licenses when building a closed-source commercial app.
- Keep a list of third-party libraries and their licenses.
- Preserve license notices for dependencies.
- Be careful with GPL and AGPL dependencies in proprietary apps.
- Treat LGPL as usable but worth checking carefully.
- Prefer dynamic linking for LGPL libraries when possible.
- Do not assume "free" means "no obligations".
- Check the exact license of the library version being used.

## Short Summary

Permissive licenses such as Public Domain, MIT, BSD, and Boost are usually easy for personal, commercial, and closed-source use.

LGPL can often be used in closed-source apps, but it has extra requirements around modifying and relinking the LGPL library.

GPL and AGPL are open-source-friendly but usually not suitable for proprietary closed-source applications unless the application is released under compatible terms or a separate commercial license is used.
