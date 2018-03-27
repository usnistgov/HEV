#
# teem: Gordon Kindlmann's research software
# Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Windows project file stuff
# 
# Makes project files for the teem dll, one for each teem binary, and
# one for the workspace containing all these

WIN32.DEST ?= ../win32/build
WIN32.TOP ?= ..\\\\..\\\\
WIN32.HEADERS ?= ../win32

project: project.build
unproject: project.clean
def: def.build

sortedObjs = $(sort $(foreach lib,$(LIBS),$(addsuffix /$(lib),$($(lib).OBJS))))
flipSlash = $(WIN32.TOP)src\\\\$(notdir $(1))\\\\$(subst /,,$(dir $(1)))

project.build: teem.dsp.build headers.copy teem.dsw.build bins.dsp.build 
project.clean: headers.clean
	$(RM) $(WIN32.DEST)/*.dsp $(WIN32.DEST)/*.dsw $(WIN32.DEST)/*.plg

teem.dsp.build:
	@echo -n "Creating teem.dsp..."
	@echo s/TEEMALLDOTC/$(patsubst %.o,\# Begin Source File\\n\\nSOURCE=%.c\\n\# End Source File\\n,$(foreach obj,$(sortedObjs),$(call flipSlash,$(obj))))/g > cmd.ed
	@echo s/TEEMALLDOTH/$(patsubst %.h,\# Begin Source File\\n\\nSOURCE=%.h\\n\# End Source File\\n,$(foreach lib,$(LIBS),$(addprefix $(WIN32.TOP)src\\\\$(lib)\\\\,$($(lib).PUBLIC_HEADERS) $($(lib).PRIVATE_HEADERS))))/g >> cmd.ed
	@echo s/TEEMALLINC/$(foreach lib,$(LIBS),\\/I \"$(WIN32.TOP)src\\\\$(lib)\")/g >> cmd.ed
	@echo "s/ #/#/g" >> cmd.ed
	@sed -f cmd.ed $(WIN32.DEST)/teem_shared.dsp.tmpl > $(WIN32.DEST)/teem_shared.dsp
	@sed -f cmd.ed $(WIN32.DEST)/teem_static.dsp.tmpl > $(WIN32.DEST)/teem_static.dsp
	@unix2dos $(WIN32.DEST)/teem_shared.dsp 2> /dev/null
	@unix2dos $(WIN32.DEST)/teem_static.dsp 2> /dev/null
	@rm -rf cmd.ed
	@echo "done"

headers.copy:
	@echo -n "Copying headers..."
	@mkdir -p $(WIN32.HEADERS)/include/teem
	@$(CP) $(foreach lib,$(LIBS),$(addprefix $(lib)/,$($(lib).PUBLIC_HEADERS))) $(WIN32.HEADERS)/include/teem
	@echo "done"

headers.clean:
	$(RM) -r $(WIN32.HEADERS)/include/teem

teem.dsw.build:
	@echo -n "Creating teem.dsw..."
	@echo s/TEEMBINPROJECT/$(foreach bin,$(BINS),\\nProject: \"$(bin)\"=.\\\\$(bin).dsp - Package Owner=\<4\>\\n\\nPackage=\<5\>\\n{{{\\n}}}\\n\\nPackage=\<4\>\\n{{{\\n\ \ \ \ Begin Project Dependency\\n\ \ \ \ Project_Dep_Name teem_static\\n\ \ \ \ End Project Dependency\\n}}}\\n\\n\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\\n)/g > cmd.ed
	@echo "s/ #/#/g" >> cmd.ed
	@sed -f cmd.ed $(WIN32.DEST)/teem.dsw.tmpl > $(WIN32.DEST)/teem.dsw
	@unix2dos $(WIN32.DEST)/teem.dsw 2> /dev/null
	@rm -rf cmd.ed
	@echo "done"

bins.dsp.build: $(foreach bin,$(BINS),teembin.$(bin).dsp.build)

# NB: pattern-specific variable BIN is set once per binary
teembin.%.dsp.build: BIN = $(patsubst teembin.%.dsp.build,%,$@)
teembin.%.dsp.build:
	@echo -n "Creating $(BIN).dsp..."
	@echo s/TEEMBINNAME/$(BIN)/g > cmd.ed
	@echo s/TEEMALLINC/$(foreach lib,$(LIBS),\\/I \"$(WIN32.TOP)src\\\\$(lib)\")/g >> cmd.ed
	@echo s/TEEMBINDOTC/\# Begin Source File\\n\\nSOURCE=$(WIN32.TOP)src\\\\bin\\\\$(BIN).c\\n\# End Source File\\n/g >> cmd.ed
	@echo "s/ #/#/g" >> cmd.ed
	@sed -f cmd.ed $(WIN32.DEST)/teem_bin.dsp.tmpl > $(WIN32.DEST)/$(BIN).dsp
	@unix2dos $(WIN32.DEST)/$(BIN).dsp 2> /dev/null
	@rm -rf cmd.ed
	@echo "done"

def.build: $(WIN32.DEST)/teem.def

$(WIN32.DEST)/teem.def: teem.dsp
	@echo "Creating teem.def..."
	@msdev $(WIN32.DEST)/teem_shared.dsp /make "teem_shared - Win32 Release" /clean
	@-msdev $(WIN32.DEST)/teem_shared.dsp /make "teem_shared - Win32 Release" /build
	@dlltool --export-all-symbols --output-def teem.def $(WIN32.DEST)/shared/release/*.obj
	@grep -v '; dlltool' teem.def | grep -v '_real' | grep -v '??' | grep -v '_airThreadWin32Body' > $(WIN32.DEST)/teem.def
	@rm teem.def
	@-msdev $(WIN32.DEST)/teem_shared.dsp /make "teem_shared - Win32 Release" /clean
	@echo "done"

win32:
	@-msdev $(WIN32.DEST)/teem.dsw /make all /build

win32.clean:
	@-msdev $(WIN32.DEST)/teem.dsw /make all /clean
