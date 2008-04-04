# -*- mode: makefile; coding: utf-8 -*-
# Copyright © 2005-2007 Jonas Smedegaard <dr@jones.dk>
# Description: Check for changes to copyright notices in source
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# TODO: Save scan as "file: match" (needs rewrite of main loop in perl)

# TODO: Save scan as "file (license): match" (needs /usr/bin/licensecheck from kdesdk-scripts)

_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class

ifndef _cdbs_rules_copyright-check
_cdbs_rules_copyright-check := 1

include $(_cdbs_rules_path)/buildcore.mk$(_cdbs_makefile_suffix)

# Space-delimited lists of (regular expressions for) files to ignore
# (use DEB_COPYRIGHT_CHECK_FIND_OPTS directly for regexes containing spaces)
DEB_COPYRIGHT_CHECK_IGNORE = 
DEB_COPYRIGHT_CHECK_IGNORE_REGEX = debian/.* \(.*/\)?config\.\(guess\|sub\|rpath\)\(\..*\)?

DEB_COPYRIGHT_CHECK_FIND_OPTS = $(DEB_COPYRIGHT_CHECK_IGNORE:%=-not -name %) $(DEB_COPYRIGHT_CHECK_IGNORE_REGEX:%=-not -regex '%')
DEB_COPYRIGHT_CHECK_EGREP_OPTS = --text -rih '(copyright|\(c\) ).*[0-9]{4}'

pre-build:: debian/stamp-copyright-check

debian/stamp-copyright-check:
	@echo 'Scanning upstream source for new/changed copyright notices (except debian subdir!)...'
	find * -type f $(cdbs_copyright-check_find_opts) -exec cat '{}' ';' \
		| tr '\r' '\n' \
		| LC_ALL=C sed -e 's/[^[:print:]]//g' \
		| egrep $(cdbs_copyright-check_egrep_opts) \
		| sed -e 's/^[[:space:]*#]*//' -e 's/[[:space:]]*$$//' \
		| LC_ALL=C sort -u \
		> debian/copyright_newhints
	@if [ ! -f debian/copyright_hints ]; then touch debian/copyright_hints; fi
	@newstrings=`diff -u debian/copyright_hints debian/copyright_newhints | sed '1,2d' | egrep '^\+' | sed 's/^\+//'`; \
		if [ -n "$$newstrings" ]; then \
			echo "Error: The following new or changed copyright notices discovered:"; \
			echo "$$newstrings"; \
			echo "Trying to locate the files containing the new/changed copyright notices..."; \
			echo "(Strings part of binary data you need to resolve yourself)"; \
			find * -type f $(cdbs_copyright-check_find_opts) -exec grep -F -l -e "$$newstrings" '{}' ';'; \
			echo; \
			echo "To fix the situation please do the following:"; \
			echo "  1) Investigate the above changes and update debian/copyright as needed"; \
			echo "  2) Replace debian/copyright_hints with debian/copyright_newhints"; \
			exit 1; \
		fi
	
	@echo 'No new copyright notices found - assuming no news is good news...'
	rm -f debian/copyright_newhints
	touch $@

clean::
	rm -f debian/stamp-copyright-check

endif
