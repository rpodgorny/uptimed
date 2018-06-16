## 1. Introduction

Uptimed is an uptime record daemon keeping track of the highest uptimes a
computer system ever had. It uses the system boot time to keep sessions
apart from each other. Uptimed comes with a console front-end to parse the
records, which can also easily be used to show your records on a web page.

The original author is Rob Kaper but since version 0.3.4 I (Radek Podgorny)
took over the maintainership as Rob is overwhelmed by work (and so am I
but I try my best). Remember, we're talking about about maintainance only.
Still, feel free to contribute anything, branching and merging is not
a problem.

## 2. Availability

You can always get the latest release from the Uptimed github repository:

https://github.com/rpodgorny/uptimed/

## 3. Acknowledgments

Uptimed was inspired by a similar utility called 'ud', but is completely
different by design. Instead of using PID files to prevent multiple
instances, Uptimed uses the system boot time to seperate log entries from
each other. This is believed to be more reliable when switching runlevels or
accidentely killing Uptimed or running multiple instances of it.

For a list of contributors to Uptimed, please read the CREDITS file.

## 4. Contacting the author/maintainer

Questions, comments, bugfixes, patches: Radek Podgorny <radek@podgorny.cz>

Original maintainer and author: Rob Kaper <rob@unixcode.org>
