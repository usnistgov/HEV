/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
/* This file was originally written by Eric Tester.  Many
 * modifications have been made by Lance Arsenault.
 */

static char *eric[] = {
  const_cast<char*>("   100   136      201            2"),
  const_cast<char*>(".. c #050505"),
  const_cast<char*>(".# c #636363"),
  const_cast<char*>(".a c #2d2d2d"),
  const_cast<char*>(".b c #444444"),
  const_cast<char*>(".c c #4f4f4f"),
  const_cast<char*>(".d c #2f2f2f"),
  const_cast<char*>(".e c #4d4d4d"),
  const_cast<char*>(".f c #141414"),
  const_cast<char*>(".g c #757575"),
  const_cast<char*>(".h c #222222"),
  const_cast<char*>(".i c #434343"),
  const_cast<char*>(".j c #585858"),
  const_cast<char*>(".k c #5b5b5b"),
  const_cast<char*>(".l c #3a3a3a"),
  const_cast<char*>(".m c #525252"),
  const_cast<char*>(".n c #272727"),
  const_cast<char*>(".o c #343434"),
  const_cast<char*>(".p c #898989"),
  const_cast<char*>(".q c #1a1a1a"),
  const_cast<char*>(".r c #4b4b4b"),
  const_cast<char*>(".s c #7c7c7c"),
  const_cast<char*>(".t c #5e5e5e"),
  const_cast<char*>(".u c #646464"),
  const_cast<char*>(".v c #696969"),
  const_cast<char*>(".w c #404040"),
  const_cast<char*>(".x c #292929"),
  const_cast<char*>(".y c #101010"),
  const_cast<char*>(".z c #474747"),
  const_cast<char*>(".A c #535353"),
  const_cast<char*>(".B c #3d3d3d"),
  const_cast<char*>(".C c #7f7f7f"),
  const_cast<char*>(".D c #707070"),
  const_cast<char*>(".E c #363636"),
  const_cast<char*>(".F c #5e5e5e"),
  const_cast<char*>(".G c #2f2f2f"),
  const_cast<char*>(".H c #696969"),
  const_cast<char*>(".I c #272727"),
  const_cast<char*>(".J c #3e3e3e"),
  const_cast<char*>(".K c #565656"),
  const_cast<char*>(".L c #1b1b1b"),
  const_cast<char*>(".M c #636363"),
  const_cast<char*>(".N c #484848"),
  const_cast<char*>(".O c #767676"),
  const_cast<char*>(".P c #4f4f4f"),
  const_cast<char*>(".Q c #525252"),
  const_cast<char*>(".R c #454545"),
  const_cast<char*>(".S c #151515"),
  const_cast<char*>(".T c #242424"),
  const_cast<char*>(".U c #3c3c3c"),
  const_cast<char*>(".V c #4e4e4e"),
  const_cast<char*>(".W c #434343"),
  const_cast<char*>(".X c #0e0e0e"),
  const_cast<char*>(".Y c #5c5c5c"),
  const_cast<char*>(".Z c #787878"),
  const_cast<char*>(".0 c #313131"),
  const_cast<char*>(".1 c #6b6b6b"),
  const_cast<char*>(".2 c #4d4d4d"),
  const_cast<char*>(".3 c #5f5f5f"),
  const_cast<char*>(".4 c #1f1f1f"),
  const_cast<char*>(".5 c #2b2b2b"),
  const_cast<char*>(".6 c #4a4a4a"),
  const_cast<char*>(".7 c #5f5f5f"),
  const_cast<char*>(".8 c #8d8d8d"),
  const_cast<char*>(".9 c #3d3d3d"),
  const_cast<char*>("#. c #262626"),
  const_cast<char*>("## c #5f5f5f"),
  const_cast<char*>("#a c #1d1d1d"),
  const_cast<char*>("#b c #3d3d3d"),
  const_cast<char*>("#c c #696969"),
  const_cast<char*>("#d c #646464"),
  const_cast<char*>("#e c #656565"),
  const_cast<char*>("#f c #515151"),
  const_cast<char*>("#g c #545454"),
  const_cast<char*>("#h c #333333"),
  const_cast<char*>("#i c #2b2b2b"),
  const_cast<char*>("#j c #8c8c8c"),
  const_cast<char*>("#k c #595959"),
  const_cast<char*>("#l c #5a5a5a"),
  const_cast<char*>("#m c #747474"),
  const_cast<char*>("#n c #5e5e5e"),
  const_cast<char*>("#o c #4c4c4c"),
  const_cast<char*>("#p c #393939"),
  const_cast<char*>("#q c #494949"),
  const_cast<char*>("#r c #424242"),
  const_cast<char*>("#s c #505050"),
  const_cast<char*>("#t c #353535"),
  const_cast<char*>("#u c #6b6b6b"),
  const_cast<char*>("#v c #464646"),
  const_cast<char*>("#w c #181818"),
  const_cast<char*>("#x c #464646"),
  const_cast<char*>("#y c #474747"),
  const_cast<char*>("#z c #7d7d7d"),
  const_cast<char*>("#A c #303030"),
  const_cast<char*>("#B c #0a0a0a"),
  const_cast<char*>("#C c #979797"),
  const_cast<char*>("#D c #6d6d6d"),
  const_cast<char*>("#E c #212121"),
  const_cast<char*>("#F c #393939"),
  const_cast<char*>("#G c #242424"),
  const_cast<char*>("#H c #6c6c6c"),
  const_cast<char*>("#I c #757575"),
  const_cast<char*>("#J c #797979"),
  const_cast<char*>("#K c #555555"),
  const_cast<char*>("#L c #878787"),
  const_cast<char*>("#M c #686868"),
  const_cast<char*>("#N c #494949"),
  const_cast<char*>("#O c #333333"),
  const_cast<char*>("#P c #313131"),
  const_cast<char*>("#Q c #525252"),
  const_cast<char*>("#R c #1d1d1d"),
  const_cast<char*>("#S c #141414"),
  const_cast<char*>("#T c #5b5b5b"),
  const_cast<char*>("#U c #555555"),
  const_cast<char*>("#V c #818181"),
  const_cast<char*>("#W c #747474"),
  const_cast<char*>("#X c #464646"),
  const_cast<char*>("#Y c #6d6d6d"),
  const_cast<char*>("#Z c #929292"),
  const_cast<char*>("#0 c #1a1a1a"),
  const_cast<char*>("#1 c #515151"),
  const_cast<char*>("#2 c #393939"),
  const_cast<char*>("#3 c #4a4a4a"),
  const_cast<char*>("#4 c #646464"),
  const_cast<char*>("#5 c #101010"),
  const_cast<char*>("#6 c #565656"),
  const_cast<char*>("#7 c #565656"),
  const_cast<char*>("#8 c #454545"),
  const_cast<char*>("#9 c #3e3e3e"),
  const_cast<char*>("a. c #9c9c9c"),
  const_cast<char*>("a# c #676767"),
  const_cast<char*>("aa c #393939"),
  const_cast<char*>("ab c #8e8e8e"),
  const_cast<char*>("ac c #808080"),
  const_cast<char*>("ad c #616161"),
  const_cast<char*>("ae c #858585"),
  const_cast<char*>("af c #4c4c4c"),
  const_cast<char*>("ag c #878787"),
  const_cast<char*>("ah c #5e5e5e"),
  const_cast<char*>("ai c #3d3d3d"),
  const_cast<char*>("aj c #2b2b2b"),
  const_cast<char*>("ak c #444444"),
  const_cast<char*>("al c #2d2d2d"),
  const_cast<char*>("am c #737373"),
  const_cast<char*>("an c #6e6e6e"),
  const_cast<char*>("ao c #2a2a2a"),
  const_cast<char*>("ap c #262626"),
  const_cast<char*>("aq c #606060"),
  const_cast<char*>("ar c #505050"),
  const_cast<char*>("as c #252525"),
  const_cast<char*>("at c #424242"),
  const_cast<char*>("au c #212121"),
  const_cast<char*>("av c #555555"),
  const_cast<char*>("aw c #363636"),
  const_cast<char*>("ax c #393939"),
  const_cast<char*>("ay c #343434"),
  const_cast<char*>("az c #494949"),
  const_cast<char*>("aA c #717171"),
  const_cast<char*>("aB c #616161"),
  const_cast<char*>("aC c #505050"),
  const_cast<char*>("aD c #707070"),
  const_cast<char*>("aE c #4d4d4d"),
  const_cast<char*>("aF c #727272"),
  const_cast<char*>("aG c #5f5f5f"),
  const_cast<char*>("aH c #3d3d3d"),
  const_cast<char*>("aI c #292929"),
  const_cast<char*>("aJ c #2e2e2e"),
  const_cast<char*>("aK c #0e0e0e"),
  const_cast<char*>("aL c #141414"),
  const_cast<char*>("aM c #7b7b7b"),
  const_cast<char*>("aN c #797979"),
  const_cast<char*>("aO c #555555"),
  const_cast<char*>("aP c #474747"),
  const_cast<char*>("aQ c #7d7d7d"),
  const_cast<char*>("aR c #727272"),
  const_cast<char*>("aS c #626262"),
  const_cast<char*>("aT c #5e5e5e"),
  const_cast<char*>("aU c #474747"),
  const_cast<char*>("aV c #646464"),
  const_cast<char*>("aW c #818181"),
  const_cast<char*>("aX c #7d7d7d"),
  const_cast<char*>("aY c #555555"),
  const_cast<char*>("aZ c #767676"),
  const_cast<char*>("a0 c #505050"),
  const_cast<char*>("a1 c #090909"),
  const_cast<char*>("a2 c #484848"),
  const_cast<char*>("a3 c #5b5b5b"),
  const_cast<char*>("a4 c #1e1e1e"),
  const_cast<char*>("a5 c #4e4e4e"),
  const_cast<char*>("a6 c #626262"),
  const_cast<char*>("a7 c #676767"),
  const_cast<char*>("a8 c #4b4b4b"),
  const_cast<char*>("a9 c #5e5e5e"),
  const_cast<char*>("b. c #555555"),
  const_cast<char*>("b# c #838383"),
  const_cast<char*>("ba c #5b5b5b"),
  const_cast<char*>("bb c #1e1e1e"),
  const_cast<char*>("bc c #111111"),
  const_cast<char*>("bd c #616161"),
  const_cast<char*>("be c #2f2f2f"),
  const_cast<char*>("bf c #585858"),
  const_cast<char*>("bg c #6d6d6d"),
  const_cast<char*>("#r.9#p.U.b.b#r.b.z.J.b.b.J.9#r#r#r#r#r.J.b#r#r.b.z.6.6.z#r.b.b.b#o.J.b.9.R.b.\
b#p.b.z.z#o.R.b#s.b.z.9.b.9.b.b.J.6#o.JaU#g.b.R#r.z#o.R.z#9#r.9.b.z.J#p.z.b.R.\
z#p.b.z.b.9aU.9.b.R#r.z.9.z.e#r.9.9.J.b#r#r.9"),
  const_cast<char*>(".J.J.9.9.b.b#r.b.b.b.9#r.9.J.b.b.J.9.b#p.z.9.b.z.i.6.R.z.9.b#r#r.z.E#p.J.9.b.\
z#p#p#p#o.z#r.z.z.z.b#p.9.b.9.b.9.9.z.z.b.b#r.6#r#r.9.R.z.b.b.z.R#s.9.z#o.R#o.\
b.b.z.zaC.J.b#r.b.R.R#o#r.b.z.R.z.R.R#r.b.R.J"),
  const_cast<char*>(".J.9.b.J.b.J.9.b.J.b.U.6.b.b#r#r.b.b#r.b.U.U#p.b.b.R.U.9.z.b.z.z.b.b.9#p.b#p#\
p#9#p.J.9.9.R.baC.9.9.b.J.b#r.J#9.6#r.b.z.b.b#r.baU.R.9aC.J#r.R#r.9a8#o#o.z#sa\
CaO.z.z.b.z.z.R.z.b.b.R#r.b.b.9.b#r.9#p.9.R.9"),
  const_cast<char*>(".b.6.J.z#r.z.b.9.9.z.B.R#r.J.b#r.R#r.b.J.R.b.9.z.9.b#p.b.9.z.R#p.R#9.R#p.z.R.\
9a8.R.R#p#p.9#s#NaU.9.9.6#r.b.l.b.b.z.z.9.b.RaU.b.9#p.9.9.b#o#oa8aC.b.z.z#raC.\
b.z#r.b#o.z.z.z.z.R.b.9.b#s.b.9.b.9.b.9.R#o#r"),
  const_cast<char*>(".b.b.z.z#p.b#p.b#p#p.9.z.9#p#r.z.R.z#r.b#pa8.z.R.z#r.J.9.z.9#p.9.z.9.z#r.J.9.\
R.R.z#r#p.J.R#raU#r#r.9.z#p.J.9.b.b#o#o#o#o.R.R#o.z.ba8.9.z.R.R.b.za8aU#g#o.b.\
9aCaC.P.b#o#r.z#s.z.z#o#r.b.R.w.R.9.R.9.9aC#r"),
  const_cast<char*>(".J.z.b.z.J.b.b.J.b.9.z.z.z.J.z.9.z.9.b.z#9.z.z#p.b.R.9.6.z#p.z.z.9.9.R.z.9.b.\
R#p.9.9.E.9.z#r.b.R.9.z.R.z#p#r.b.RaUaC#s#o.z.z.z.z.z.z.z.b.R.9.b.zaU.6#gaC.z#\
oaC#o#oaUaCaU.z.R.b.RaC.b.z.z#raU.R#r.b#r.RaU"),
  const_cast<char*>("#p.b.J.b.b.6.J.9.b.b.R#p.J.b.b.b.b.J.z#p#paC.l.J.R.R.b.baC.9.b.b.R.9.9.b#p#s.\
9.9.9.b.9.9#r.b.9.9.z.z.b.9.z.z#r#raU.b.z#s.zaC.z.b.z.z#sa8.z.baCa8a8.zaOaCaCa\
C.zaC.zaC.baC.z.A.z#r.9#r.R.R#raU#r#r.9.R.z.9"),
  const_cast<char*>(".E#r#r.b.z.9.z.z.9.z#p.J.J#r.b.b.E#p.b.b.baU#r.9.6.b.J.9.z#p.9.b.9.9#p.6.z#9#\
p#p.9.b.b.9.9.b.J.9.b.9.R#o.z.R.b.9.6.z.z#r#r.b.b.b.b.R#r#raUaCaU.baUaCaC#o#oa\
CaC.z.b.z.zaU.baU.baU.9.9.R.PaU#r#r.R.R.9.z.U"),
  const_cast<char*>(".9.b.z.b.z.b#p.9.b.b.J.b.b.z.b.9.R#p.z.9.9.9#raU.6.b.z.b.b.b.6.9.b.z#r.9#p#p.\
b#r.b.9.b.z.z.z.R.R.z.9#r#r#s.R.z.R#saC.z.b.z#s.z.z.z.z.b.z#s.RaO#g#o#o#o.P#o#\
s.baYa8aC.R#g.z#s.R.z.b.z#raU#o.b.R.z#N.R.9.z"),
  const_cast<char*>("#p#paC.z.R#r.baC.9.b.b.9.z.b.z.z.b#9.z.9.b#o#r.R.b.9.b.z#s.R.R.z.z.b.z.R.z.z.\
R.9.b.z#p#r.b#s.z#p.ba8.b.R.9.z.z.z.z.R.R.zaC#s.R.z#r.z#o#s#g#ra2#o#g#g#o#o#ra\
C.z#oaUaC.R.e.R#s#N.Ra8.z#r.6.R.z.9.b.b.b#oaU"),
  const_cast<char*>(".J.U.b.z.z.R.z#p.b.R.6.z.z#o.R#r.b.R.i.z.z#o.z.R#o.b.z.R.R.z#s#s.R#o.z.b.R.z.\
b#9.z.R.9.PaU.b.R.b#o.R.9.b.z.6.b.z.zaO#s#o#o#o#r.b.z.R#o#o#o#oaC#oaCaO#o#o#oa\
C#o#o#g#saP.R.b#s.R#s#s.z.R.b.9#r#r.9.9aC#o.z"),
  const_cast<char*>(".J.b.b.V.z.zaU.b.J.z.R.R#o.z#r.b.z.z.R.9.z.R.R.baC.R.b.z#r#oaU#g.R.R.R.R.z.A.\
z.z.R.9#o.R.R.R.R#r.z.9.9#r#r.R#o.z.z#s.z.RaOaU.9a8#s.z.z#o#oaUaUaOaU#g#NaC#o#\
oaCaC.z#s.za8#saC#o.R#o#o.z.R.b.B.b#r.z.R#s#r"),
  const_cast<char*>(".z#r.b.z.R.z.b.J.U#9.z.b.baC.b.9.9.9.b.R.9.b.b#raU.b.z#r.b#oaU.w#r#r.z.z.b.9.\
9.z.J#o#r#N.b#r.b.b.9.z.R.R.z#s.za8.6aCaU.z.z.baC.z.R.R.e.R.R.z#s.ba8#oaC#s.z#\
oaC#oaC#r#r#s.b#s.RaU.6.z.R.z#s.b#r.b.w#r.b#o"),
  const_cast<char*>(".b.b.zaC.b.R.R.z#9#9#9#o.b#gaU.z#9.b.z#r.b.J.b#r#r.z.R.z.zaC.z.9.b#r#o.b.R.z.\
R.b.b#raU.b.9.9.b.9.9.R.B#t#A#t.w.z.wax#pax.R#o#r.9.b#s#r#o#s.R#s.z.z#o#o.za8.\
z.z#oaUaUaY.b.z#s.9a8#o.R.b#o#o.RaU.z#p.z.RaU"),
  const_cast<char*>(".baC.baU.b.z.z.9.z.z.z.Ra8.9.b.9.z.9.b.b.z#p.z.z.b.b.R#s.9#o.z.B#r#r.b.R.b#s.\
R.9#p#p#t#GaIbbbbbbbb#Gbb#w#w#w#waIbbbb#w#w#GaI.G.9.b.b#o.b.z.R.R.z.R#o.R.9#s.\
z#o.b.6aU.za8.za8.R.b.b.z#s#s.R#r#r.9.b.R.b#r"),
  const_cast<char*>(".b#o#raUaC.R.b.9.b.9#o.R#o.b.9.6.9.b#p.z#p.J.z.9.z.R.b#r.RaU.R.9.b.z.z#r.z.Ra\
Jbb#w.L.S#waK.S.S.y.S.y.y#w#w#S#w.y#w.S#w#w.y#w#wbbaIaJ.9.z.R#o.z#o.RaC.z.z#o#\
oaU.baC#s.z.z#s.z.Ra8a8#sa8#ra8#r#r.b#N#raU.9"),
  const_cast<char*>("aC#g#o#o.b#o#r.R#r.b.R#r#r.b.z.z.9.9.b.9.6.9.6.R#r.b#r.9#r.b.b.b.R.z.b.z.R#w.\
S.y.y.S.S.S#B.S.S#Sbb#w.S.y#w.S.L.y.y.4#w.S#B.S.S#w.y#wbb.R#s.b#o#o.z.RaC.z.b.\
baC.R.R.zaC#raC.R.z.z.z.z#o.9aC.R#o#o.z.b.6.b"),
  const_cast<char*>(".b.baC#r.z.b#r.b.9.9aC#r#o.b.9.b.9.i.9.b.R.9.9.z.R#r.b.b.b.ba8.z.b.E#paJ#w.S.\
S.S#w#S#w.y.S#B#w#S.f#Sbb.y#w#w#B.4#w.y.y.S.y#w.y#B#waK#w#w#A.R#o.z#r.b#oaC.za\
8.b.b.z.z#r.b.z#r#s#o.z.z.b.z.R.9.R.R#s#o#o.z"),
  const_cast<char*>("#r.baC.b#o.9.9.b.J.9.b#r.z.b.b.z.z#s.R.9.9.6.9.b.R.ba8.6#oaC.z#p#9.B#t.S.S.S.\
S.y.S#w#w#w.y#wbb#G#t#p#mbb#w#D.e#w.y.S.y.y.y#w.y#w#B#w.S.y.S#G.R#o#raU#o.z.9a\
C#o.RaUaC.z#s.9.b.P#o.R.b.9.z.R.z.9.9.z#o.R#o"),
  const_cast<char*>("a8.b.9.baC#r.b.9.J.R#r.9.b.z.J#9.9a8.R.9.b.z.z.z.R.b#p.z.baC.b.b.V#t.S.S.S.S.\
S.S#B.S.R.5.B#s.B.9afafab#A.y.eac#t#saj#w.y.y#w#B.y.y.y#S#w.y.S.G.9.zaU.b.R.z#\
N#o.b.R.zaC#r.z.z.baUaU.b.R.9#9.9.b.9.b#r#s#N"),
  const_cast<char*>(".b.R.R.z#q.b.b.9.z.b.b.J.b#o.z.b.6.9.z.9.z#sa8.b#r.z.b.b.R#o.9.b.Rbb.y.y.S.y#\
waxao.Raca6a6#naO.W#A#e.M.W.S.R#nao#n#n#s.W#B#B#w#B..#B#w.y.y.f.SalaC#oaCaU#o.\
z.R.z.z.zaO.R#o#r#o.z.z.b.6.6.baC.b#r.b.z#o#s"),
  const_cast<char*>(".z.b.z.baC#r.b.z.J.R#r#r#r#o.R.b.b.za2.z.9.b.R#r#r#o.R#o.b.RaC.b#t.f.S.y#B.La\
j#n#s#s#naT.jaTaB#e.z.O.OaT#5#s#n.5aO.R.DaP.B#wau.y#BaK.y.y.y.y#Sbb.RaUaUaC#o#\
o.R.z#saUaU.J.z.z.z.R.R#saf#s.b.RaU.J.9#r.zaU"),
  const_cast<char*>(".b.6#r.b.b.b#p.J.9.J.J#r.J.b.z.9.b.b.b.b.b.b#r#oa8.b#r#raU#r.b.U.I#5#w#Bbb#p.\
Bax.WaIa.#D.e#e#V.O.j.e.wa6..afa6#A#p.R#ebba6#GaO#w.S.S...y#B.S#B.yax.ba8.z.z#\
s.z.z#o#g.z.R.z.z.R.R.b.Ra8a8.zaUaU.z.9.z.z.b"),
  const_cast<char*>(".6.J#9.6.z.z.b.9.z.J.b.J#raU.JaUaU#r.9.z.b.RaU.b.z.b.z.b.baUaU.0.L#w#wbb.zaf.\
WaO.O.e#n.z#A#Gat#FaiataI.f#B.Sbbbb#s.z#n.S#e.eaX.5#B...y.S.S.y.S.y.S.R.z#saP.\
9.z.b.b#sa8.b.Ra8#o.RaC.z.b#r.9.b.R.z.9.R.R#N"),
  const_cast<char*>(".laU.z.z.b.z.9.R.9.9.b.b.b.b.b.b#r.9#o.b.9.b.b.z.R.R.b.9.9#pbbbb.y.y#saO.B.9a\
Jao.S.Sbbbb#hai#h#x#h#h#F#3.0.L.S#w#B#G#s.p.H.W#m#G.y#B.S#B..#B#B.y.y#t.z#s.P#\
r.b#s.zaC.b.9.b#s.z.z.z.b#s.R.z.R.z.9.9.R.z.z"),
  const_cast<char*>(".b.6aC.b.b.b.b.b.b.9.b.R.b.z.R#r.9.9.z.z#r.9.z.b.9.b.b.b.baI#w#B.S.y.Wa6#A.5.\
S.S.S.y#hai#haw.x#F#Gai#v.c#o.L.y#B.S.S#Rax#tbb.Dbb.yaK#5#5.y#B#5.S#B#w.9.P#ra\
U#r.z#s#g.6.z.z.z.b.R.z.z.z.9.z.9.b.z.R.9.b.z"),
  const_cast<char*>(".9aC.baU.6.J.z.9.b#p.b.b.zaC.b.b.b.J#r.z#r.9.z.R.b.z#r.R.w.S.y.y#B#B.B.z.S#w.\
y.S.y.L.c.r#h.x#3.a#8#UaGad.P.L.S#w#w.y#B.Sbb#w#A#w#w.y#w#B#w#B.y#B#w.Sao#raU.\
b#r.b#o.z.z.z.R.z.b#r.z.z#p.6.9.R.R.R.z.b.b.z"),
  const_cast<char*>(".b.b.b.6a8.9.J.b.E.J.J.J#N#o.z.z#r.9.R#o#oaCaU.b.9#r.9.baJ.S.S.y#B.S#w#5#w.y.\
y.S#B.y#Fa9a0a5#T#3bf#Ta0#Nbb.S#w#S.y#w#5..#w#w.L.L.y#S#S.S.S.y.y.S.y#S.S.9.z.\
R.J.9aC.z#o.z#o.R#o.R.b.z.6.b.9.R.R.z#p.9#s#r"),
  const_cast<char*>(".b.J.6.J.z.9.z.E.9#raU#r.ba8.9.z.b.b.z.baC.J.J.b.9.b.z#paJ.L.y#B.S#B#B.y.S.y.\
y.y.S.y.y.L#E#h.9.x.x#G#E#w.S#B.S.y#w.S.y.S.S.y#B.y.S#B.S#B#B#w.y#S.S.S.y#A.b#\
o#o.z#o.z.b.zaU.b.b.9.z.z#r#r#r#r.U#9#p.ba8.z"),
  const_cast<char*>(".b#r.b.b.b.b#p.9.J.J.J.laU.b.9.6a8.z.baC.b.z.b.z.R.b.b#r.G.y#w.y.S#B.y#w#w.y.\
y.y.L.L.f.y.S.y.S.S.Sbb.S.S.S#w.y.y.S.S#w.y.S#B.S...y#w.y.y#B#B#B.y.S#5.S.S.9a\
UaC.R#s#raU.b.b.b.b.J#q.b.R#r.b#r.9.9.J#p.b#s"),
  const_cast<char*>(".9.J#r.9.b.b#p.J#p.9#p.9aC#ga8.b#r.9.b.baCaU.6aC.b.6.b.b#A.y#B.y.S#5#B.S#w#w#\
w.y.y.S.y.L.L#w.f.L#w.L#w#S.L#w#B#wbb.y#w#B#B#w#w.y.y.S#S#w.y.S#w#B.S#B.y#w.E#\
r#o.z.R.b.R.z.b.R.i.b#r.P#r#r.9.6.b.l.9.R.z#o"),
  const_cast<char*>(".J.9#r#r.R.z.9.9.J.J.b.z.ba8.b.z.b.6.z.b.zaC.b.z.zaC.b.RaJ.S.y#B#w#w.S.y#S#S.\
y.S.y#w.f.S#S.S.f#E#w#w.f#w#w#S.L#0#0.S.S#B.S.y#w.L#S.L#S#B.y#B#BaK#w#w.y.yaJ.\
b.R#oa8.z.z.b.b.b#raUaU#o#r.b.b#r.b#r.9#raC.b"),
  const_cast<char*>(".J.9aU.b.z.9.b.9.b.R#p.9.z#saC.z.z.b.za8.6a8.z.baC.z.z.zax.S#B#S#w.S#S#S.L.S.\
S.L#w.L.L.L#wbb.L#wbb#wbb.L.f#w#S#0#w#w#w.S#S.S.S#S#w.4#w.S.y#w#w#S#w.S.S.Sbb#\
s.R.z#saC.R.R.b.9.b#o.R#r.z.b.R.9.9#r#r.waU#o"),
  const_cast<char*>(".b.b#o.b.R.9.z.R.R.R.R#N#s#oa8#raC.zaC.zaC.z.z#s.ba8.Ra8#t.y#w#B.S.S.L.L#S.Sb\
bbb.L#0aI.L.L#w#w#w#0.4#w#G.f#S.S#w.S.y.Lbb.S#w#S#w#S.y#w#w.y.y.y#S#S#w.y.S.L.\
R.R.z#sa8.b#saC#r.9.z.z.9.b.R#raU#r#r.b.9.z#s"),
  const_cast<char*>(".b.z.9.b#r.9#r.b.9.J.9.laUa2#o#o#o.baUaCaC#oaC#g#o#oaU#o#G#E.f#5.S.y.f.f.f#w.\
S#w.y#w#w.L.S.S#w.L#w#w#w#0au.y#w.y#w.S.S.y#w.y.S#w#w#w.L#w.L#w.y#w.y#w.y#S.S.\
0#gaC.z.b#o.b#o#o.R#o#o.R.z#oaUaU.R.9#r.9.z.6"),
  const_cast<char*>("#p.z.9#r#r.9aUaU#r.9.9aUaCaCa2aCaCaC.6#qaCaUaU#k.b#o.b.zbb.S.y.y.L#S.L.Lbb#w.\
S.L#w#S.L.Lbb#wbbbbbbbbbb#w#S.4#w#G.f#0#wbb.y#w#w.y#w#w.y#B#w#w.y#w.S.y.y#w.Sa\
I#oaC.z.z#oaC#o.R#o#o#o#r.b#q#raU#r.9#r#raCaC"),
  const_cast<char*>(".RaUaC.b#o.J#r.9#r.J.J.9.baEaCaCaC.z.baU.6aC.z.6.ba8#o#o.L.f.L.f.Lbb.L#w.f.L#\
0#G.LbbbbaIbbaobb#GaIaIbbaIas#0.f#w#w#w#w.y#S#w#S.y.y.y#w.S#B.y.L#w.S.L.y#w#S.\
S#r.zaC.z.6.z.zaU.b#N#r#raU.9aU#r.9.9#3.9#r.b"),
  const_cast<char*>(".6.b.baC#r.b#r.J.J.J.9.9aCaCaCaC.6.zaC.b#o.b.b.Ra8#o#r.Rbb.S.S.S.ybb#w.S.S#Eb\
b.Lbbbb#wbbbbbbbb#GaI#GaI#Gas#0#w.f.L.y.S#w.S.S#w.S#w.y#w.S#w.S.f.y.y#w#w.y#w.\
f.ba8aCaCaC.b#o.z#o#o#qaU#o#o#raU.9aU.l.9#oaC"),
  const_cast<char*>(".b.b.6.b.6.z.z.b.9#r.b.J.baCa2aUaC.b.6.baC.b.b.b.z.6.R#O.L.S.y#w.y.L.L.Sbb#wb\
b#wbbbb.hbbbbaobb#G#G.4bbbbaIbbbbbb.L#w.y.y#w.S.y.y#w.y#B.y#B#w.y#Sbb#w#B#w#Sb\
b.9aY.b.6.b.z.z.z.z.z#raUaC#o#o#r#r.9#p.w#gaO"),
  const_cast<char*>(".baU.b.6.b.U.b.6#r.P.J.baUaC#oa2.zaY.6#p.z.b.z.6.V.6a8#A#w#B.y.S.Lbbbbbbbb.S.\
S.S.h#G#.aJ.hbbbbbb#Gbb#G.5bb.h.S#w.S.L#0#0#w.S.S.S.S#5#5aK#5#B#B.y#w#w#B#w.L.\
S.Ea2#qaU.b#g.R.b.z.za8.9.z.9aC.b.z.R.R.R.P#g"),
  const_cast<char*>(".baC.6.6.6.R.b#oaU.J#q.b.P#o.K.baU#g#gaU.z.z.za8aC#s.zbb.y.y.S.LaI#0ao#R#5#R#\
R#w#G#G#E#GaI.GaJaJbbaJbbbb.SaK.S.Xa1#wbb#G#S.S.S#B.y#B.S.y.S#B#B#B#B#B#w#w.y#\
waIaC#o#o.z.z.b#r#r.b.z#oaC.b#r#o#r#r#raw.zaC"),
  const_cast<char*>(".b.b.b.b.b.zaU#g.6aUaUaU#oaC#raU#o#oaU.b.J.b.6aO#o#p.B.L.f.L.L.L#wbba1#5...X#\
B#5.Sbbbbbbao.GaJaIaI.G.h#R#R#5bc#5.S.X#w#Gas#S#S#w.y#B.S.y.y#w.y.S.S.y.y.S.y.\
y#G.A.za8.zaC.b.J.b#o.z#gaC#gaUaCaU#o#r.b#s#s"),
  const_cast<char*>("#g.z#o#oaC#oaC.R.baUa5.baU#o.b.b#ra2#l.b.b.z#oaUaU.6.R#w.Lbbbbbb.S........#B.\
.....#5.SbbaJaJalawaJ#Gbb#Rauaubb#R.5bbaoaI#G.4bb#w#S#w#w.y#0.S.y.Sbb.S#5.y#B#\
wbb.6#o.b#s#o.z.baC.b.z.baCaU.b#raC#o.9.R#g#s"),
  const_cast<char*>("a2.baCaUaUaC#qa2aU.z.z.b#oaC#r.baUaC#q#raC.z.b.b.6#o#pbb#waI.4.q..a1#B#5#5..#\
B#5.S#5#wbbao#GaI#AaIaJ#A.G#AaJao.GaJ#A.5#A#G#AaI.L.f.S.L#0#w#w.ybbbb.S#5#w#B.\
ybb#o#s#NaUaC.P#o#o#saC.baUaCaUaUaU#o#N#r#o#o"),
  const_cast<char*>("#q.ra5aU.b#o.baE.b.6.R.baCaU#o.baC.RaU#o.R.b#r.z.6aC.9.h.f#A#w.Sa1#5.S#w.Sbb#\
R#RbbbbbbaIaIaI#A#t#A#Abb#t.G#A#GaJbb.hbb.Sbb#A#Gasbb#w#w.L.y.L.S#w.S#5.S.S.y.\
ybb.A#o#o#saUaC.z.z#o.R.b#oaU.baU.zaUaC#r#s#o"),
  const_cast<char*>(".PaUaC.baC.b.b.KaC.b.baCaU#r.P#o#o.baCaUaU.b.b.baC#o.9.L#w.4.S.S#w#R.h#R.I.h.\
I.h.h.5aobb#AaI#A.G#AaJ#A#G.G#G#Gbb#R#w.S#w.S.Lbb.5bb.Lbb.L.Lbb.SbbaI.h.q#w#w#\
E.R.k.7.AaUa8.z.z#s.P#g#o.PaU.9.baC#oaU#o#s#o"),
  const_cast<char*>(".KaCaC.zaC.b.P#gaUaUaC#o#g.RaO.P#o.R#oaC#N#o#r.zaU.P.G.f#A#5.Xbb#O.haoaIau#w#\
wau#w.S.Sbbbb.G.GaI.GbbaobbbbaIbb#wau#w#5aKaK#wbbbb#A.G#Gasbbbb#Abbauao.IaJ.za\
7aFaSaO.PaUaC.baCaU#gaC.baC.b#raC.z#oaU#g#s#g"),
  const_cast<char*>("a2a2#oaCaUaC#g#gaCaCaY#l#g#r#r.b.z#o.z.RaC.b.b.b.z.z#Abbbb.SbbaoaJbb#w.S#5#5#\
B.y#w#5.S#RbbaI#N.R.w.Gbb.S#Gau#w#wbbaKa1..#5#w.S.SbbaJ.G#A#Abbaobb#w#AaJ.Rb.a\
TaO.9aw.w.w#oaU.6aC#o#o#o.R.V.9#o#o.z#o#oaC#g"),
  const_cast<char*>("#g#o#o#oaYaC.6#laC.6#kbdaC.J.b#o.b.b.b.b.b.b.b.baC#o.R.R#p.S.h.h.5.5#waK.S.S.\
.....aKaK.S.S.9.Aar#o#t.hbb#R#w#w#w#w..a1..#5.y#5a1#wbbaJ#t.5bbaobb#R.h#A#s#s#\
s.PaOaU.9.R#o#o.b.b.zaUa8.6.z#o#o#saU.6aO#r#o"),
  const_cast<char*>("aC.KaC#g#saYaC#obaaC#kaY#q.9.R.R#r.z.b.b.b.z.baC.b.z.w.R#s#OaoaJbb.SaK#5au#w.\
...#5.S#R.S.h#oaS#D.k.RaJaobbaI#GaIbbbb#R.haIbb#G#R#Rbb.5#A#A.5bb.h#Rbb#AaOaO.\
k.gaFaS.AaS.A#o.R.baC.6.RaU#gaC.z#o.z#oaO.R#r"),
  const_cast<char*>("#g#o.KaC.K#g#g#gbd#gbd#g.6.b.z.b.R.b.z.b.R.6.b.b#o#o#t.G.B#p.0.Gbbau#waubb#R#\
wbb#wbb#Rbb#taT#H#DaT.P.R#t.G#AajaIaxaJaxaJax#A#AaoaIaoaIaj#A.5#AaJayax#p.Rak.\
H#z.g.7aP.H.k#o.z.za8.baC#o#saC#oaO#l#g#g.R#r"),
  const_cast<char*>("#g#gaCaU#gbdaY#g#l###g.K.b.b.R#o.b.R.9.b.9#oaC.b#s#s.9.R#s.w#Obb.5bb.h.h.5.Ga\
I.G.G.G.G#A.Ga7aF#H#k.A#s#x.w.Raw#p#p#t#t#t#GaI#t#A.naI.naj#AaI#t#t.w.GaP.R#2#\
saF.H.AaraF.k#o.z.z#saUaCaUaC#oaOaOb.#l.P.R#x"),
  const_cast<char*>("aC.KaCaCaCaC.KaCaEaCaCaC#r#r.baU#oaU.z#o#o#oaUaCaU#gaT#g#H#s#OaJ#GaoaoaJaI#ta\
o.h#O#O.9.9.9a#.HaZaS#6ak.2.RaraO#saPaP.wax#A#t#A#G.GaoaJ#A#A.G#t#t#p#Oa8.R#k.\
7aS.HaS.k#m.b.b.z#N.P#g.z#o#gaOb..P#o#o#g#r#r"),
  const_cast<char*>("aYaYaC#o.K#qaUaC#g.6aO.P.b.baU#r#NaUaUaCaUaC.b.b.9#oaS.ka7.z#taobb#AaI.Gal#t#\
2.w.R.R#o.R.Aa#aD#HaT.P#o.RaO.7.7.k#kaO.kaC#s.RaHal.w.G#G#O#OaJax.R#p#pa6aT.Z#\
D#H.Zb..7a7.A.z#raU.b.A.z.b#o.P.kaCaC#q#qaUaU"),
  const_cast<char*>("#1#1ba.KaC#gaC#q#o#o#1aC.baUa8#o.b.b#o#r.baCaUaC.b#oaT#ka7.e#Aao.h#G#tax.w.R#\
N#N.A.A#o.R.P.7aDaDa#ar.P#3.k.t#Ha7#H#Ha7a7aT.7.A.R.w.0#t#tax#t.E.B#p.Ba7.1#za\
g#DaZ.kaT#saC#s#o#o#o#o#o#o.b.b.6aUaUaCaU.b.b"),
  const_cast<char*>("aCaC#g#o#o.KaCaU#1aCaCaCaU.6aU#o.b.zaC.b.baUaCaUaU#ob..ta#aC.Gao#i#O.l#o.A.7#\
kaSaTaSaO#s.7.t#W#H#H.7ar#N#ga##Da7.g.Z.Z.g.ZaDaD.k#s.z#p#9#p#p.B#p#p.w#Da7.Z#\
z#W#HaOaO.b.zaC.P.P#gaU#oaC#o.baCaCaCaUaUaC.R"),
  const_cast<char*>("a2.r#ga2a5#o#oaCaEa2.PaU.PaCaU.b.z.z#gaC.baC#o.zaCaC.baO#D#s#t#taJ#p#o#k#Ha7.\
Ha##H#k.2#saSaD.gaF#H#T.P.2.A.k.7.taZ.gaZ.gaM.g.g.1a7#s.b.R.e.B#pax#t.9a7aO#Da\
F#D.k#o#r.baUaC#N.R#o.b#g#o#o#o#o#oaCaC.b#oaC"),
  const_cast<char*>("#1.r#o.K#1#o#oa5a5.P#g.P#o#g#o.z.PaC#g#o#g#s#s.b#q.baCa5bdaO#t#t.waO.7#D#W.ga\
Z#Ha7.k#g#s.7.HaW.Z#HaTar.A.A.2.7aS.7.HaZaW.gaW#z.g.1a6.t#s.z.R#p#p#t#s.ta7.Z.\
t#k#s.R.9.b.b.b#raUaUaUaCaO#s#oaC.b.6.b.b.PaU"),
  const_cast<char*>("#g#gaCaU.K.K#g#r.r#q.baCaCa2#q#oa2a5#l.KaC.z#gaC.Pa2a5.z.PaO.R.9#oaS#D#W.gaFa\
D.7#kaO.P#oaS#z.8#z.H#Hb..2#6#6#N.P.ka##H.H.gaZ.Z.Z#Da6.taC#k.RaxaJaJ#sa6.Z.t.\
t#DaO.9.9aw.z.RaU.A.PaO.P#oaC.z.b.zaC#s#g#o#o"),
  const_cast<char*>("aCaUbd.KaU.ba5#1a2.K.ba2a5#q.ba2#o#oaUaCaCaUaUa8.b.b.z#o.z.R#s.RaO#H#daD.gaD.\
u.7.7.ka7b.#TaD.g.g.H.7a5aS#k#s#N.A.7.7.k.k#D.g.gaA#D.t#k#ka8.R#paJaJ#s#DaA.1a\
7#g#p#p.9.z.R#o#o#o#oaU#o#s#o.z.z#o.baC#g#o#o"),
  const_cast<char*>("aCa2#gaYaC#q.P#oa5.baCa2aC.b.Pa2a2aC.baCaC.baC#o.6aC.b#o#o.z.z.b#k.taDaDaD#H.\
t.u#kaO#o#N#s#H#H#H.k#o.R.9.w.w#2.7#T.7b.#g.7#D.Z#D#D.ta7#s#s.R.d#O#OaOaD#D.ta\
Y.B.9.9.z.b.z.z#s#o#g#o#o#o.zaU.6aUaCaUaU#g#o"),
  const_cast<char*>(".K#o.K.KaCa2#1#o#o#gaC.b.KaU.6aCa2aU#qaU.b#o#o#o.zaU.baC.b.z#s.R#k.t#H#D#H.7.\
7b..7.A.G#Aal.R.A.A.w.GaJ#Gal.G#xaSa#.7aS.2#kaSa7#D.t#T#saOa8.B.d#taJ#s#DaS.R.\
B.0#p.E#p.b.z#o#oaC#oaUaC#gaUaC#o#oa5aC.b#g#o"),
  const_cast<char*>("#g#gaYaCaY#o#o#1#gaUaC.b#oa2.baCaUaCaC.b.baU#o#o#o.6.z.b#s.z.z.zaC.ta7.Ha#.t.\
7#T.7.k#t#A#G.G.Baxax.Galal.R#x#saSaT.7a7.H.7aO.7a7.t#g.6a8#s#p.G.daJaJax.wala\
lax.0.R.9.R.9.z.z.baCaCa2#oaCaCaC#gaUaU#g#o#o"),
  const_cast<char*>("#o.K.t.K#g#gaU.6.KaC.baC#o.K.z.b#oaU.ba2a5aU#o#oa8aC#q.JaC.b.6.za8a8a6a7.H.t#\
T.7#k#k#g#Naw.GaI.xal#tak.2.AaSaT#kaS.ta7#H.taO#la7.t#la8aO#say.B#t#A.hbbaJaJ.\
0.x.w.9.b.b.z.z#s#o#o.K#qa2#oaUaCaU.b.baC#saU"),
  const_cast<char*>("aCaC#o#g.P#1.KaCaCaCa2aCaCaCaUaC.b.b.b#q#oaCaU.baC#N.PaUaCaCaC#paO.Ra6.t.7.ta\
7aOaSaO.k.kaO#xawax.R#s.7.Ha7a7.7a7a7aSaT#k#D.u#H.t.uaC#saO#s.B#paJao.5aobbaJa\
J#p#p.9#r#r.9.z#oaUaUaC#o#1#o#gb.aCaU.KaU.b#o"),
  const_cast<char*>("aC#oaE.KaC.KaCaU.KaC#o.K#gaC#qaC#r#o#q#qaU#qaCaUaU.baU.b.P#r.b.R.V.zaOaS.ta6#\
kb.aT.k#k.7.7a#.7.7ah.H#DaF.H.HaSaS.kaO#k#k.ta7.t#D#k#k.9a6#s.R.B.GaoaJbb#Abb.\
IaJ#p#p#r.9.z.b#oaY.b#gaUaC#gb.#oaU#s#o#g#oaU"),
  const_cast<char*>("#1ba.KaY.KaCba.K.K#g.KaCaUaUaC.PaUaUaU.b.b.baUaU.b#o.baUa2#r#r#9aOaYaObda6aYa\
OaOaO#garaS.7aSaSa7aSa#aS.A#N#s#N#x.Raw.R#ga7.ta7.1.v#s.za6.z.B.B#Abbbbaoao#A#\
A#t#t#O.9#p.9.z#oaC#o#o#g#oaO#o#g#gaOb.aCaU.6"),
  const_cast<char*>("#1.K#l#gba.KaCaY#o.KaC#l#oa2aCaC.b.b.b.R.baUaCaUaU#g.b#o#q.b#r.baO.t#k#e#D.t.\
t#N.R.w.0#2.9#F#x#x#N.k#s.R#2.w#2aIaI#waIaO#Da7.1aAa7.RaOaT.R.R#tao.hbbaJbb#.#\
.aJ.E#t#p.9.b.z#o#g#o#oaCaC#o#gaC#o.P#gaU.b.z"),
  const_cast<char*>("bd#g#g.K.K.K.KaY#1a2#laYaCaUaCaU.b#o#o.R#raUaU#o.b#N.z#o.b.J#r.R.za6aO.1aA#Db\
d#g#t.f.4#t.waC#za6#mabb##m.p.1.1.z.G.9.7a7a7.1.gaDa7.RaT.t.B.BaJao.h.h.h.h.ha\
JaJ#t.E#p.9.9.b#N.z#gaU#gaUaCaCaC#g#g#gaC.R#r"),
  const_cast<char*>("#l#laC#o#o#l#g.KaCa5aU#g.baCaUaU#3aU.b.z.b#q#r#o#N.b.b.b#r#r#r.b#9.t#l.t.g.va\
T#HaO#Gbba8.D#m.pb#b##Cabacb#.1a7aO.2araS#Ha7#Da7a7#g.za6aO.9.B.dbb.hbbbb.haJa\
o.G#t#t#O.J#p.9#r#N#g#o#g#g#gaU.K.baCaCaC.R#r"),
  const_cast<char*>("#g#ga5.KaEaC#1#1aC.K.K.K.P#o.r#q#qa2.b.b.b.baU.b.PaUaU#o#qaU#r.9.zaY#s#gaA.1a\
6aT.7.2#N#s#o.D.C#z#m.CaA#DaTaSaS.7.7.7aOaS#HaA.g.t.R#s#e#s.B#paJ.h#R#Rbbaobbb\
baJaJ#O.E.9.9.R.z.z#s#oaCaCaCaC.P#o#ga8.z.b#s"),
  const_cast<char*>("#g#1.K#1a2#1aE#g.Ka5#o.K#g#o#oa5#oaU.baC#g#g.PaUaUaUaU#g.z.J.6.9#p.Ba6aO.1#m.\
1a6.t.A#N#6ar.2#6.7aSaSaSan#4aSaS#6.7aO#Ta7.Z.g.1a6.RaOa6.Bax.BaIaI#RbbaJbb.h.\
IaJaJ#O#O.0.R.R.R.9aC#s.z#o#gaCaOaU#g#g#g#NaU"),
  const_cast<char*>("#gaC#g#laCaE#1#1#1aEa2a5a5a5.baC.b.baU.6.baU.P.P#r.ba8.b#o.9.z.b#p.9aC#sa6.1#\
D.7aOar.7.2ahahah.k#HaQaZ.Ha7aSaS.k#k.k#k#D.gaA.1a8.Ra6aC.B#t#t#Abbaoaobb.h#Ra\
JaoaI.E.0.9#r.R.9.b#s.z#s#o#gaCa8aC#o.k#ra8.z"),
  const_cast<char*>(".Ka5#gaCa5.K#oba#q#o.K#gaC.K#o.b.baU#oaU.z.baUaU#qaU#o.9#o#o#r.9.9.E#p.R#s#ea\
A#D.7.P.Aarar.7.kahaSa#a#aS.7.7.7aS.7.7#H#D.gaAa6.RaO#k.B.B#A#taJbbaobbbbaobba\
JaJaJ#t#t.9.9.R.R#N#o.RaCaC.6aC.b#gaC.baUaCaU"),
  const_cast<char*>(".Kba.K.KaU#l#o#gaEaEaC#g#g#o#o#q#q#q#qaCaUaC.P.P.b#oaO.R.9.9.9#p.zaJao.W.Ba6#\
D.vaS.k.Rar.k.k#kaS#k.7#kaTaS.7aS.7#T.3.v.1aA#D#k.zaOaf#taJ.najbb#Rbbao.h.hbba\
JbbaJ#t.E.9.9.R#r.b#o#g#o#raU.baCaU#oaC#gaC#o"),
  const_cast<char*>("#g.K.K.Ka5a5.KaCa2#o#gaCaOaC#q.6.baC.6.b.z#o.PaU#q.baU.b#p.6.b#p#p#OaJaJ#p.z#\
ka7a6.k#k#g#saT#k.k.k.7.7aS.7#H.7.k#k#H.1#DaA.t.R#s#s.B.5#Aaj#taJ#G#Gbb.hbbaJa\
J.h#A#t.x.Eaw.9.9.R#o#N#N.P.K#gaUaCaCaU#g.PaU"),
  const_cast<char*>("a5aC#g#g#o#g.K#g.K.K.K#oa5.PaC.baC.b#raC#o.z#gaC.b#o#r#r.9.9.9#p.9.9#taJ#A#9.\
za7#D.v.kaOara7.kaOaS.t#Ha7aS.k.taS#D.1#DaA.taC.9a8.Bax.G#A#taJao.S#R#Raobb.Ib\
baJaJ#O#t.9.B.9.9.R#r#o#oaU#gaUaCaUaC.P#g#oaC"),
  const_cast<char*>("#g#gaEaC.K#g.K.PaC#q#oa5.P#qa5#q.b.6#p.b.6.zaCa2aUa5.b.b.b#p.9#p.9.9.G.haJ#t.\
Ba6a6aT.taSaT.1.1a7#H.va7.ZaF#H.u.u#D#D.1.v#k.EaP.B#p.daJ.GawaJ#R#5.h.IaJbbaoa\
o.h#O#O#O#t.l.9.l.E.9.b#o.zaUaCaU.KaUa2a5aUaU"),
  const_cast<char*>("aCaE.maCaCaCaU#o#o#o#g#o#laU.b.b.JaC.J.baUaU#g#o#q.P.J.b.b.9#r.9.9.E#taobbao.\
0#s#k.t#Ha7.H.H.g#D.Z.Z.Z.Z.Z#D#HaDaDa7aA.t.6#p.z.BaxaJ.IaI#t#p#R#w.qao.Ibb.h#\
R.h.I#taJaI.0#t.J#paUa8#o#saU.KaUaC#gaUaC.P.b"),
  const_cast<char*>(".Ka5.Ka2.K#gaC#oa2#1#o.KaU.b#o#r.baUaUaU.K#r#r.PaUa2aU#r#r.b.9.9.9#taIaJ.Sao#\
t#p#sa6.t.u#H#HaF.g#z.Z#z.g.gaD#D.gaA.t.taO.B#p.B.Bay.n.5#t#p.Rbb#5...h.N#A.h#\
R.hbb.5ao#taJ#O.9.9.9.R.9aCaUaCaUaC#o#gaC.baC"),
  const_cast<char*>("#g.K.P.P#o#g#o.K#o.KaCaCa2#o#q.baUa2aC.b.baU#g#g.P#oa2aU.b.b.z#p.E#O.EaJ#wbba\
J#A.B#s.7.va7.H.Z#z#z.Cae#z#z#D.1#Da6.t.V.BaJ.B#t.G#A.G.G#p.z.zaoa1#w.T.W.eax#\
G.5aJaobbaJaJ.E#p.b.J.9.z.zaCaU#o#o.PaU#g#o#g"),
  const_cast<char*>(".PaCa2.K#1#o#o#o.K#g.K.KaC#qaUaUaC#q#qa2aC#qaUaU#oaUa2.b.b.baUaU#p#t#paa#5bb#\
taJ#p#p#ka7a7.H.Z#z#z#zae#z.Z#D.H#Ma6#s.BaJaJ#paJaJaJ#t#p.z.z.e.h.X.Sao#yaOafa\
Iao.haobb#OaJ#O.E#p.J.9.b.9.zaCa5aU#o#o#l.P#g"),
  const_cast<char*>("#g#l#q#o#o.K#g#o#gaUaC#g#oaUa5#q.baUaUaUaC#r.P#oaCaU#o#o#o.b.b.b.9.9azaoa1.5#\
t#taJ.B.R.taT#D#D.Z#z#m#z#z.Za7a6a7#say.Iay#paJ#t#t.w#p#s#kaO#s.5#w#B#G.Qaf.b#\
pbbaIao.haJaI#O#p.E.J.z.9.6#r#o#o#g#ob..P#N.R"),
  const_cast<char*>("#g#o#o#g#g#g#g#gaU#gaC.PaC#o#o#o.PaU.b.baU#q.baUaCaU.baC.b#r#r#r#pafafaJ#5ao.\
b.B#AaJ.B.B.ta7#m.1.DaA#D#D.Ha6#k#s#p#t#tay#AaJ#t.w.w.zaTaT.t#s.B.S.S.5.e#s.W.\
B.Ibbaobb.haJ#t#p.R.B.z#p.b.z#gaC.zaUaC#g.z#s"),
  const_cast<char*>("a5a5#g#oa5#g#g#gaC#obd#gaO#oaUaU.6aU.baCaC.baCaU#o.P.b#r.b#raU.R.Bafaf.B#5#A#\
s.zaa.I#P.I.e#s.ta6aSa6aT.taT#s.RaJaJaJ#paJax#Aax.waPa6a6aTa6#n#9aK.S#y.Q.jaf.\
eaJbbbbbb#t#A#t#t.w.R.E#r#r#o#o.AaC#oaC#g#o#o"),
  const_cast<char*>("#g#o#1.K#g#gaCaU#oaCaUaCa2aC.J.b#o.KaUaCaC#q.baU.P.b.z#r#r.R#r.z.V.Q.Qay#BaJ#\
s.e.R.BaJ#A#A#9aP.R#y.R.R.B.B.B#A#AaJ#A#p.5aJ#p.R#s#naT.1a7.t.t.Bbeao.Qaz#y.Na\
f.W.h.hbb#O#t#paJ#p.w.9.z.9.b.z.b#o#r.P#gaCaU"),
  const_cast<char*>("a5.Ka2a5#o#o#oa2a5.r#laCaUaC#o#1.KaC.b#r#r.b#raCaC.b.z.baU#r.R.e.W.Q.j.e#5#p#\
kb.#s.eaI#t.BaJaJ.B.B#p.d#O#9#Aay.d.B#p.B.B#p#paf#k#e.va6a6aTaO.9#paf#s.j.W.W.\
Vaf.B.Sao#Gal#t.E.E.J#9.z#paCaU#gaUaU#gaO#saC"),
  const_cast<char*>("aC#oaE.6.KaCaC#oa5#oaC#o#o#o#l.b.baU.z.baU.b.baC.b.z.b.R.R.9.9.j.j#n#n#yaJ.B#\
ka6aO#s.B#taJ#t#A.B#p#p#Aayay.dayaJ#Aay#tax#9.R#l#ea7#D#D.1a6a6#p#AaO.j.j.Qaz.\
W#Q.zayaJ.x#O.E#p.9.J.b.b.b#g#o#gaCaCaU#o#g.R"),
  const_cast<char*>(".KaE#1aEaCaUaC#oa5.K#1a2#1#o#o.baCaU.b.b.b.b.z.b.z.b.R.9#s.R.Q.Q.j.e#n#s.W.B#\
ka6aTa6a8.Bax#AaJ.B#p#p#9ay#paJaJ.B#p#t#p.R.Ra6#D#D#m#D.1.1.ta6.z.WaB.jaf.e.Wa\
z.N.j#saJ#t#p#t#p.J#r#r.z#NaO#o.6#gaCa5a5#o#o"),
  const_cast<char*>("#g.KaC#q.6a2#o#o#qa5.ba5aCaUaU.b.R.b.9.z.b#r#r.R.b.za8.j.jaz.j#s.jaO#n#naf.9a\
6a6#e.t#ka8#p#t#p#tay#p.B#A.B.Bax.B.B.B.9.b.t#D#D#maA.1.1a6#eaO.j.Y#na6#nafaO.\
W.Q.j.e.R#p.0.E#p.9.z#r.R#N.P#o#o#o.P#o#o.P#q"),
  const_cast<char*>("aC#g#oa5a5a5.KaUa2a5.ba2#qaCaC#o.b.b.9.z.b.J#r.b#n.t.j#n.j.jaf.j.Q.Q#n.YaO.Wa\
Oa6#D.1.t#g.R#p.BaJ.B.9#9.B#p#p#p#p.B.Ba8#sa6aA.1#D#maA.1.1a6#n.Q.Y.Y#n#n#s.W.\
jaz.Q.j.j.e#p#p.J.9.9.9.baUaUaC#o#o#r.2#o#o.K"),
  const_cast<char*>("#l#o.Ka2#o#1aC.KaC.b#1#1.b.b.b.b#o#r#r.9aOaOaO.Yafaf.e.j.j.j.j.W.YaO.j.j.Q.ia\
Ca6#D.1#e.ta8.R.R#p#p.R#p#p.e.R.B.z.Ra8aY.t#e#z#maA.1.1.Da6aOaq.Q#QaO.QaO.j.Q.\
Naf.j.Y.j.Q.QaO.Q.9.b.zaC.b.JaU.baUaCa5a2#qaU"),
  const_cast<char*>("#o.ra5#qa2aC#qaU.6.KaU.P#q#ra2#r.baCaO#Q.j#Q.j.Y.Q.Qa3aO.Q.Y.Q.Q.Qaz.W.Y#eaO#\
s#l#Y.1.t#e#s.z#s#saO#n#saO#k#saO#saY#k#l.t#maA#m#m#IaA#eaT#n#n.j.j.Y.F.Yaf.Q.\
Q.Q.Y#n.Q.Q.Y.YaB#n#saO.9#r#o.baUaCaUa2a5.PaC"),
  const_cast<char*>(".K.r#oa2aU.6#qa2.ra5#q.r.b#qaC.b#s.Q.j.Y#Q.Y.Qa3#7#Q.Q#Q.YaO.j.Q.Q#Q#Qa3.F#n.\
Va6.t#YaA#e#s#s.R.eaOaT#e#ea6aTaO#ka6a6.t.1#Y#m#m#m.1#m#e#e#n.Y.jaf.j.jaO.Q.Q.\
Y.e#n.Yaf.j.j.j.j.Y.Q.FaCaC.z#g#o.baCa2#q#o#g"),
  const_cast<char*>("#o#o.P#oaC#q#o#q.baUaCaC.VaY.V#n#Q.Q.Q#Q.Y.Y.F#Q.Y.Q.F.YaO.YaOaB.j.j.Q.F.Y.Y#\
na6#eaA#IaA#e#s.V#saOa6#m#m.Ha6aT.ta6.t.1.1.C#z#m#m#e.M#M#eaB.Y.j.Q.j#n.Y.Q.j.\
e.Y.YaB.W#n.Y.jaO.F.QaO.Y#QaOa8aC.baUa5aC#oaC"),
  const_cast<char*>(".KaU.b#gaUaU.b.ba8aOaY.QaOaz.jaz.YaO.j#K.Y#QaB#Q.j.Q#Q.Q.Q.F#naO#Q#Q.jaB.Fa3#\
naOa6.1aAaA#e#kaf#gaOa6.H#DaT#Ma7a6.v.1.1#m#z#m#I.D#D.M.MaV.F#n.Q.j#naOazazaf.\
j.Y.#.j.W.Y.j.j.j.YaBaO.F.F.j.Qaf.b#o#o.PaUaU"),
  const_cast<char*>("#g#g.b#q.b.b#s#n.Y.Y.j.Q.j.j#f.Y#Q.F.QaO.j.Q.Y.YaB.Ya3.Y#Q.Q.Q.Qa3.Q.j#e#e#7a\
B.jaOa6.1.ZaA#e#k#n#naO.t#k#ea6a6a6.1#M#m#m#z#m#z.1#e.M#n.Y#QaB.Y.eaVaB.W#Q.Y.\
Y#QaV.Y.j.M.j.Q.j.Qa3.FaB.Q.Q.Qa6a6aO#g#gaUaC"),
  const_cast<char*>(".b.6.z#9.VaOaz.j.j.jaf#Q.j.Y.Q#Q#n.Y.Y.j.j.Q#Q#QaV#n.Y.Y.Y#QaB#n#QaB.YaO.F.F.\
F.YaOa6#eaAaA#ea6.t#naT#l.ta6a6#e#e.1#m#maA#IaA#Y#ea6#e.Y.j.j.YaO#n.j.j.j.j.j.\
j.j.Y.W.Q#y.F.YaV.F#n#n.j.F#f.F.F.j.Q.Q.Q.zaC"),
  const_cast<char*>("#naO.e.jazaz.j.j.Q.V#naO.Q#Q.FaB.jaO.Q.W.Y.jaV.Y.Q.F#Q.F.QaB.Y.#.Y.YaO#caf.j.\
N#P.W.Y#n.1aNaA.1a6a6a6.1a6#e.1.1#m#D.D.ZaA#m#eaO.j.M.j#n.Y.Q.YaB.Yafaf.j.Y.Ya\
B#c.F.j.Y.Q.j.j.j.Q.j.Y.Y.jazaB.M.j#y#yaO.eaO"),
  const_cast<char*>("#n.j.Q.Q.Y#Q.jaO#Qaz#Q.F.j.Y#Q.Y.Y#Q.Y.Q.F.Y#naB.F#n.Y.Y.Q.F.F.Y#n.Y#n#n.Q.5#\
a.T.W.Y.Y.F.D#m.D#D#e#D#D#DaA.D#m#maA#m#m#Y#Y#naq.M#c#QaV#e.Y.Y.F.j.N.j.YaB#n.\
Y#e.F.W#n.Y.j.j.j.F.YaB#n.Y.Y.F.j.Y.Q.Q.Y.Y.e"),
  const_cast<char*>(".j.j.j.Q#7az#Q.Y.j.j.Y#Q.Y.V.j.Y.YaV.Y.j.j.Y.Y#naBaB#Q.YafaV.Y.Y.j#n.YaO.Ta1b\
c.5#b.Q#y#n#nac#z#maA#Y.D.g#m#m.D.1#I.O.D#n.j.j.YaBaVaV#n.M.M.YaB.j.N.j.Y#n#n#\
QaV.Yaf.Y.j.Y.j.j.Y#naBa6.Y.j#n.F#n.Y#n.j.j.Y"),
  const_cast<char*>(".j.jaO.Q.Y#QaO.Q.e.Y.Q.jaz.j.jaO.F#n.Y.j.Q.F#n.F.Y.F.YaB.jaBaO.j.j#n.V#P..#5.\
S.oaxax.Baf#n.Y#L#Z#Z#m#Y.1.1.1.1#I#M.j.Qaz.B.Bbe.Y.FaVaB.M.YaO.F.Y.Q.F.F#n.Y#\
M.D.j.WaB.Y.Q.j.YaV.Y.Y.Y.j.j.j.QaOaB.Y.j.Y#n"),
  const_cast<char*>(".YaBaz.Q#Q.j.j.Y#Q.Y.Q.Y#n.Y#naBaOaB.MaB.Y.QaB.YaO.F.Y.Y#naB#n.j.j.jaf#5.Xbc.\
Xau#A#9.Va6#y#y#y.s#L#L#V#Z#Z#Z.s.Y.Y#yay.h.q#a#a.T.N#naV.F.F#eaB.Y#QaV.Y.Y#n#\
caVafaf.j.j.#.Y.MaB.QaB#Q.Y.j.YaBaBaB.YaV.M#n"),
  const_cast<char*>(".Y#QaO.j.Y#n#n#Q.Q.Y#QaV.Y.Q.Q.F.F.Y.F#c#c#Q.Q#Q.Y#c.F.Y.Q.Y.Y#QaB.W.oa1a1a1#\
5ao.T.Y.Fa8.Q.Q.jaf#c.O.Oac.s.Y.W.W.Wa4a4.Taoaoap#aau.W.Y.Y#Q.FaVaBaV#c.Y.YaVa\
V#c.N.Q.Y.j.Y.F.Yaz#f.Y.Y.M.##naVaBaVaBaV#caB"),
  const_cast<char*>(".Qaz.jaz#Q.j.Qa3.Q.Y#e.j#Q.FaBaB.F#Q.Y.j.YaB.Q.jaVaV.Q.Y#Q.Y.Y#n#n.W.qbc#5ao.\
W.j.Q.5ao#A#yaf.Y#A#u#J#J#Laaaa.Q.Way.5#A.W#P#P.Iau.T#5be.Y.Y.Q#n.M#c.Y#e#e#n.\
M.M.W.j.Y.M.M.Y.j.j.Y.M#caB#naB#n.MaBaV.MaV.Y"),
  const_cast<char*>(".j.j.Q#Qaz.j.Q.Y#7.Q.Q.j.Y.Y#Q.MaB#n#n.Y#Q.FaO.YaBaV.F.FaBaBaO.Y#Q.oaK#Paa.W.\
F#Q.j#baoao.Waa.W.N.Y#L.OaV.W.N.Waa.o.5#A#A.oao#Aao#GaL.q.W.Y.Y#n.M#u#c.M.Y.M.\
Y.M.N.Y.MaV.YaO#f.jaV#c.MaB#e#n.MaVaB.YaB.M.M"),
  const_cast<char*>("aO.QaO.j.Q.j.Q#f.FaB.j.YaB.Y#naBa3aB.Y.Y.FaB.Q.Fa3aB.YaBaB.DaO.Y.N.q.q.jaz.Ya\
B.YaOafaaay.W#Xaaaa.j#j#J.Naabe.5#Aao.N#baabeaf#f#y#bau#w#w.Waq.M#u.M#c.M.M.F.\
Y#c.N.MaV.j.j.j.YaVaVaB.F#naVaV.M.F#e.Y#e#n#n"),
  const_cast<char*>("#na3.Q#Q.Y.F#c#c#n.Y.YaV.F#7az.F.FaBaB#naB#eaO.j.Y#c#n.Y.Y#caBaz.5aK.oaOaz.j.\
j.Y#Q.j.W.z#b.W.Y.jaf.s.F.W.Taoapao.o.W.W.W.j.j.j.j.W.o.q#w#5.o.M.M.D.Y.M.YaV.\
YaV.YaVaV.j.Q.Y#c.MaVaV.YaqaV.YaV#naV#naV#n#c"),
  const_cast<char*>("aBaB.Q.Q.FaB.Y.FaBaB.Y.YaB.Y.F.F.j.Y#Q.FaV#c.F.YaV.Y#n.YaBaB.Y#P.S.haf.Y.j.Y.\
YaB.#aVaV.Y.j.W.5bebe.j.j#f.Q.Waa.Waf#n.Q.Q#c.FaVaV#n.j.NapaKaK#R.Q.M.M#c.YaV#\
n#c.Q.Y.Q.Q.j#naV#c#c.Y.YaV.Y#caV.YaV.Y.Y.Y.Y"),
  const_cast<char*>(".Y#n.Q.F.F#Q.Q.j.Y#QaB.Y.F.Y.j.j.Y#naV.Y.Y.Y.V.Y.F#Q.jaq#naO#P.qaa.Q.YaB.M#c.\
YaVaV#caV.j.j.j.Yaz.N.j.N#f#y#K.jaf.WaVaf.Y#naB.M.F.YaV.Y.Q.5#5.q#a.T.j.Y.M#c#\
c#c.j.Y.YaV.F.FaV.M#eaV.M.F.MaV.MaV#n.Y#n.j.Y"),
  const_cast<char*>(".Y.Y.F.Y.F.Y.jaq#n.YaBaBaB.F.j.Q#caBaV.Y#naBaV.F.F.YaV.Y.Yaaao#X.j.Y#Q.M.D#e#\
naV#caV.YaBaV.j.Yaqa6.jaz.Y.Q.Y.M.Q.Y.Y.Y.MaVaV.MaV.Y.Y.Y.Y#Q#b.oao#a.qap#A.e.\
j.O.Y.M.Y.M.MaV#c#c.M#c.Y#n#caV.M#MaV.j.M.D.Y"),
  const_cast<char*>(".Y.YaB.j.YaB#n.Q.MaB#c.F.Y.Y.F.Y.Y.Y#e.MaB.F.Y.YaB.MaVaB#b.qap#yaB.MaV#e#c.Y#\
n#naVaB.F.Y.Y.j.e.j#n.W.Q.W.Y.Y.e#y.M#n.Y.DaB.YaB#c.M.#.Y.Y.Y.Y#n#y#b#a.q.Taoa\
y.Q.Q.YaV.M.MaVaVaVaVaV#caBaV#caV.M#c.Y.MaV.M"),
  const_cast<char*>(".j.YaB.jaO#n.Q.YaBaVaV#naVaO.Y.V.j.Q.Q.F.M.#.Y.Y.j.Y.j.Fao#a.z.W.Y.j.Y#naV#n#\
n.j.YaBaBaB#e.Y.Q.Y.Q.j.Q.Y.Qaz.N.jaV#n.M.M#c#n.FaV#uaV.##n.Y.D.M#u#u.W.o#R.T#\
Aao#y.Y.DaV#c.D.M#c#c.M#c.M#u#c.DaV#u.M#c#u#c"),
  const_cast<char*>(".Q.j#n.Y#Q.Y#7aO.Y#n.MaB#n.F#c.Y#QaB.YaBaV.MaB#naBaO.Y.Yaz.W.j.F.W.Q.Y.FaB.j.\
Y.F#QaV#e.Ya3.F.Y#n.j.j.YaB.j.j.Y.YaVaB#eaB#e#c.M.M.DaV.Y.Y.MaVaVaVaV.Y#X.dao#\
Rau.B.Q.Y.M.Y.M.M.M#c#u.M#c.M.D#caV#c.D#caV#c"),
  const_cast<char*>("az.j.Q.F.Y#Q#ea6.j.Y.Q#eaB.YaB#c.Q#Q.F.Y#c#caV.j#n.Y#Q.Y.YaB#caV#fazaz.QaBaBa\
B.Y.Y.Y#e.M#c.j.Y.Y.Y.Y.jafaz#QaB.DaB#Q.Q.FaBaV#caV#caV.M#caV#uaVaV#n#n#n.Y.Wb\
e.Tapaf.OaV#c.MaV#uaV#c.M.Y#n.DaVaB.M#c#u#n.Y"),
  const_cast<char*>(".Y.j.j.jaz.j.jaz.Q#QaB#c#naB#caB#n#n.M.M.D.M#caV.Y.F.Y.Y#Q.Y.D#eaB.j.W.jaB.ja\
BaB#e#e#uaV.M.MaV.Y.Y#n.j.e.QaBaB#c#u.YaV#c#c#MaV#uaVaV#uaV#uaVaV#n#n.Y#e#c.Ya\
Obe#b.Q.Y.M.Y#c.#aV.Y.YaV.YaVaV#caB#c.D#c#u.M"),
  const_cast<char*>("aB.Y.j.j.Faz.F.j.j.Y.Y#e#n#eaVaBaB#n.Y.M#c#uaVaB.j.j.FaV#c.Q.F.MaB.#.Y.Q.Y.j.\
YaVaB.j.Y.jaVaV.YaV.Y.Y#s.W.Y#c.D.D#caV#c#u#c#M.M#u#caVaVaV.M#c.Y#Q.j.F#uaVaV.\
#.Y.W.FaBaVaBaBaV#caV.Y.Y.YaVaV#c#caB#c.M#ubg"),
  const_cast<char*>(".F.F.j.j#c.j.Y#e#n.QaBaBaB.F.YaVaV.Y#naVaB#caB#c.Y#n.Y#c.Ya3aB#eaBaB.Yaz#naV#\
c.M#caB.j.YaBaV.Y#f.j.Yaz.j#Q.D.D#caV.M#u#u#uaR#c#u.MaV#c#c#c#c#M#M.Y.Y#u#c#c#\
uaV.#aVaV#c#c#c#M#u#c#M#naV#caV.M#caV#caB#c.D"),
  const_cast<char*>("aBaB.j.YaBa3#QaB.YaO.F.Y.F.#.YaB#ca6.Y#eaBaBaV#caVaB.Y#eaB#ca3#n.YaBaB.j.Y.Fa\
VaVaVaVaV.Y.YaV.MafaV.Y.QaV.M.D.O#uaVaVaV#u.M#u.D#u#c#c#c.M#c#c.M#c.MaB#c#eaV.\
D.MaR.D#c.M#eaV#uaV.D.D.MaVaV#c#c#uaV#c#c#c#c"),
  const_cast<char*>("#n#n.Y.Q#QaBaOa3.Y.j.Q#n.M.#aV#c#caBaO#c.DaB#c#c.M.Y.Y.F#c#caB#caBaBaV#n.Y.Fa\
VaVaVaV#caV.#.YaV.j.Y.Y.j.M#c#M.DaRaV#c.M#caV.D#u.M.D#u#caVaV#c#c.MaV.MaV.M#c#\
c#c.D.M.M.Y.M.M.D.DaV.MaV#caB.M#uaV.Y#c#c.DaV"),
  const_cast<char*>("#n.YaB#naV.M.FaOaB#c#n#n#caV#caVaB.F.j.Ybg.Dbg#u.Y#e.F.Y.FaBaB.YaB.Y.MaV.Q.Y.\
##u.MaVaVaV#n.Y.Q.YaB.Y.Y.Y#c.M.D#e#n.MaR.M#u.M#uaB.Mbg.DaB#I.O#e#c.MaB.MaVaV.\
M.D#u#uaV.YaV.M.D.Dbg.D.Y.YaV.M#c.MaV.M.MaV.M"),
  const_cast<char*>(".j.j.YaO.Q.jaB.YaB.F.Y.Y.FaV#c#caVaV.Y#n.M#c.D#c.Y#c#n.YaB#c.MaVaBaVaVaV.Y.Y.\
MaV.DaV.Y.M.Y.YaV.Y#n.Y#n#eaV#c.DaV#c.O.D.O.M#u.Mbg.O#c.MaV.OaBaB#c.M.M#c#u.M#\
caV.D.MaBaV#c.##u.O.YaBaB.Y#caV#caB#c.O#u#caB"),
  const_cast<char*>(".Y.F.Y.Y.j.j.M.j.YaBaV.Y.YaB#caV.##naV.j#c#caVaV#n.Y#n.Y#e#c#u.M#c#c#caV.Y.Ya\
BaV#caVaV.Y#n.YaVaB.Y.Y#c#c#u#c.O.##u.D#u#c#u#c.M#c#c#caVaV.OaVaB#c#c#c.D.D#c.\
MaVaVaVaV.jaV.YaV#u#c#caB.Y#c#M#M#n#c#c#caBaB"),
  const_cast<char*>(".j.##n.Y.Y.j.F.F.Q#caV.Y.j.#aVaVaV.j.M.Y#n.M#caV.MaBaB#QaBaV#c.MaVaVaVaVaB.Y.\
Y#caV.#.#.#.YaV.M.M.M#c#Y.O#c.D#c.YaV.D#u.D#c#c#u.D#c#M#c#c#caVaV.F.FaV#u#u#c#\
u#c.D#c#caVaV.YaV.M#caV.FaV.MaV.M#c.D#c#caV.M"),
  const_cast<char*>(".YaV#n.MaV.j.Y#c.jaBaV.MaV.Y#c.YaB#c#c.M.#.YaV#caBaV.YaV#c.YaV.FaVaV.Y.M.M.F.\
Q.#aVaV.Y.YaVaV#c#e.j.j#n.D#cbg#e.Y#c#u#u.D.D.M.O#u.M.MaV#eaB#uaBaV.M#u#c#c#c.\
D#u.M#u.M#u#caV.#aV#u#uaV#c#c.M#c#caVaV#c.M#c"),
  const_cast<char*>("aO.F#c.Y.F#Q.j.F.FaOaVaBaB.F#naBaB#c.MaB.YaBaV.MaBaB.F.Y.F.Y.MaB#ca6aVaV#c.#.\
W.#aVaVaVaV.#.Y.Daf.W.Vaz#c#ubgaVaV#c.Y#naV#c#c.D.D#c#c#caVaVaB.M#c#c#u.D.DaV.\
D.M.O.D.D.M.M#caV.Y.O.D.M.M.DaB#c#u.MaV.M#u#c"),
  const_cast<char*>("#n#c.Y.YaBaV.jaf.Y.Q#c.F.YaV.j.F#cbgaV#c#n.F.M#caVaV.Y.Y.#.YaVaBaBbg.M#c.#aV.\
j.jaV.DaV.Y.MaV#eaOa6#saf#M.D.D#caV#c.D#c#uaV.M#ubg#c#u.MaV.DaV#c#c#u#c#u#uaV#\
c#uaV.MaVbg.OaVaB.##MaV.M#c.D#I.M#uaVaB#I#caV"),
  const_cast<char*>(".YaBaOaBaB.YaB.Y.FaVaVaV#n.Y#naV#caVaB.F#n.QaVaVaVaBaBaV.F.QaBaBaB#n.Y.Y.jaV.\
j.Y#e.Y.M.M#caV#c.F#Y.Fa6.O.D.MaVaV.D#u.M#c.M.F#caVaV.D#c.D#c.D#c#cam#u.D.O#c.\
M#c#uaV#uaV.M.F.M.D.MaV.O#u.Y.O#c#c#c#u#u#u.M"),
  const_cast<char*>(".YaBaVaB.Y#Q.Ya3#QaVaO.F.FaB#n.M#c.FaBaBaB.j#cbg#caV.Ya3aBaBaVaBbg.F.F.#.Y.j.\
Y#n.D.MaV.##M#caB.F#e.F.D.M#c.OaVaV.O.DaVaVaB.M#u#c.M#c#cbg.D#c.M#c#u.D#c#u#ca\
V.Y.Y.MaVaV.F.YaB#c#c.D#u.M#c#c#caVaV#c.D#caV"),
  const_cast<char*>(".YaB.YaB.Y#Q.FaB.Y#Q.j.F.F.F.##caBaBaV#uaBaB.Y#caV.F#n.YaB.Y#naV.M.DaVaV.j.Y.\
F.j.YaV.F.Y.Mbg.DaB#c#Y#c.D.O#u.M#u#u#u#u#caV#u.O#uaVbg#ubgbg.DaV#c#u#u.M#c#ca\
VaV#n#c.Y.Y.YaB#c#uaV.Y#c#u.M.O.Y#M#M.OaR.M.Y"),
  const_cast<char*>("#Q#QaBaVaBaV#Q.F.F.j.Y.j.YaVaB.MaB#naB#caVaV.YaVaV.MaB#QaVaO.#.YaVbg.M#c#n.F.\
Y.j#n#caV.##c#c.M#caV#c.D.D.D#c#c.M#c.D#c.M#c#u#u.M#c.D#c.D#uaV.O#u#c.D#c#uaV.\
Y#n#Q.Y#naVaB.M#c#caVaV#c#caVaBaV.D.D.D.DaV.j"),
  const_cast<char*>("#y#KaV#naB#c.YaOaBaB.Qaz.#aV#uaBaBaB.YaB#caVaVaB.Y#caB.YaVaV.Y#naB#u#u#e.Y#ea\
V.Y.M#caV.Y#c.D#c#c#c#c#e#c#eaVaV#c.D.D#c#caB#c#c#c#c.D.D#c#u#c.MaVaB#c.Y.MaV.\
YaV.Y.M.YaVaV#u#u.Y.M#ubg#caB#c.M#c#c.D#caVaO"),
  const_cast<char*>(".j#f.FaVaVaVaV.Y.YaBa3#Q.j#caVaV.M#c.Y#naV.MaVaBaBaVaV.M#c.YaB.D#n#caVaV#n.Ya\
B.Y#n#cbg#c.Dbg#u.Dbg.D#M.D.OaVaV.D#c.MaVaVaV#u.D#u#c#u.O#u#cbg.M#c#uaq#n.j.Y.\
F.Y.Y.YaB#c#u#c.D.D.Mbg.O.D.DaV#J.O.D#uaV.YaV"),
  const_cast<char*>(".Y.Q.F.F.#.Y#n#Q.YaBaB.Y#Q.Y.#.M#c.D#n#caV#c.M#caVaB#caV.#aB#naB.M.j.MaB#c.Fa\
V.Y.Y.M#caB#c#u#u#c.D.D#M.D#u.M#u.D.OaV#u#uaV#caVaVaV#c#c#caVaV.M.#.Y.Q.j.Q.Ya\
VaBaV.M#c#c#uaV.D.D#caV.D.M#c#c.Y#M.D#M.M.jaV"),
  const_cast<char*>("#f#fav.##n#c#eaV.Y.Q.FaB.Y.jaV#c#eaBaBaV.MaVaVaVaBaV#c#c.YaVaBaB.Y.j.Y.F#c.Y#\
QaV.jaV#caV#c.O#c#M#u.Dbg.DaR.M#u.D#caV.D.M#c#u#c.MaV.D#M#u.jaV.F.Q.Yaf.j.YaVa\
BaB.M#c#u#caV.M#c#c#caRaV#c.M.O#u#u#u#u.j.Q#M"),
  const_cast<char*>(".Y.Y.FaV.Y.F#eaV.Y.Q.Y.Y.Y.jaBaVaV#caV.Y.Y.MaV.Y.YaBaV#caVaVaV#c.Y.YaO.FaBaB.\
YaB.jaV#c#c.D.D.D.MaR.D.D#c.O#c#u.D.M.M#uaV.MaVaVaV.Q.Yaq.Y.j.j.Q.Q.Q#naV.MaV#\
c#c#u.MaV#caV#c.M.M#c#c#c#u.D.D#u#u#c#c.j.Y.M")
};
