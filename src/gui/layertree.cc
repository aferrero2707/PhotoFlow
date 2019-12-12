/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#include "../base/image.hh"

#include "imageeditor.hh"
#include "layerwidget.hh"

#include "layertree.hh"

static const struct {
  guint  	 width;
  guint  	 height;
  guint  	 bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  guint8 	 pixel_data[24 * 15 * 3 + 1];
} icon_meter = {
  24, 15, 3,
  "\376\376\376\377\377\377\376\376\376\376\376\376\376\376\376\376\376\376"
  "\377\376\376\376\377\377\376\376\376\377\376\376\376\376\376\376\376\376"
  "\376\377\377\376\375\375\377\376\376\376\376\376\376\376\376\377\377\377"
  "\376\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376\376"
  "\376\376\376\377\377\377\377\377\377\377\377\377\377\376\376\377\376\376"
  "\377\377\377\376\377\377\376\377\377\377\377\377\377\367\367\377\357\357"
  "\377\353\353\377\365\365\377\374\374\376\377\377\376\377\377\377\377\377"
  "\377\376\376\377\376\376\377\377\377\377\377\377\377\377\377\376\376\376"
  "\377\377\377\377\377\377\377\376\376\377\377\377\377\377\377\377\377\377"
  "\377\353\353\377\217\217\377HH\377\27\27\377\0\0\377\0\0\377\0\0\377\0\0"
  "\377\21\21\377==\377\200\200\377\325\325\377\377\377\377\377\377\377\377"
  "\377\377\376\376\377\377\377\377\377\377\376\376\376\377\377\377\377\373"
  "\373\377\377\377\377\356\356\377LL\377\0\0\376\0\0\376\0\0\377\12\12\377"
  "\13\13\377\0\0\377\0\0\377\0\0\377\0\0\377\0\0\376\0\0\376\0\0\37788\377"
  "\322\322\377\377\377\377\375\375\377\376\376\376\376\376\376\376\376\377"
  "\377\377\376\377\377\376\203\203\376\4\4\376ZZ\377\263\263\376\320\320\376"
  "\344\344\377\351\351\376\342\342\376\311\311\376\244\244\376LL\377\0\0\376"
  "\0\0\377\0\0\377\1\1\376\0\0\376\0\0\376gg\376\377\377\377\377\377\376\376"
  "\376\376\377\377\377\377\377\376``\376\177\177\376\377\377\376\377\377\377"
  "\377\377\376\377\377\376\377\377\377\377\377\376\377\377\376\377\377\376"
  "\377\377\376\377\377\377\361\361\377mm\355\5\5\360\0\0\377\0\0\376\0\0\376"
  "\0\0\376&&\377\364\364\376\377\377\376\370\370\377\224\224\377\346\346\377"
  "\377\377\377\372\372\377\374\374\377\377\377\376\377\377\376\376\376\377"
  "\377\377\377\377\377\377\377\377\377\376\376\376\373\373\375\374\374\377"
  "\377\377hQQ\377,,\377\0\0\377\0\0\377\1\1\377\0\0\377\37\37\376\364\364\377"
  "\332\332\377\377\377\377\377\377\377\376\376\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\377\377SSS\233\241\241\377\377\377\376;;\377\0\0"
  "\377\0\0\377\0\0\377\0\0\377!!\376\377\377\377\377\377\377\376\376\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377\377"
  "\377\377\377\377\376\376\376\376\376\376\377\377\377\237\237\237\0\0\0\377"
  "\377\377\371\377\377\377\377\377\377--\377\0\0\377\0\0\377\0\0\376ww\376"
  "\376\376\377\377\377\376\376\376\376\376\376\376\376\376\376\376\376\377"
  "\377\377\376\376\376\376\376\376\377\377\377\376\376\376\376\376\376\377"
  "\377\377\332\332\332\0\0\0zzz\377\377\377\375\375\375\376\377\377\376\355"
  "\355\376\0\0\376\32\32\377\322\322\376\377\377\376\376\376\377\377\377\376"
  "\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376\376\376"
  "\376\376\377\377\377\376\376\376\376\376\376\377\377\377\25\25\25\0\0\0\330"
  "\330\330\377\377\377\377\377\377\376\376\376\376\377\377\376\311\311\376"
  "\377\377\377\377\377\376\376\376\376\376\376\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377"
  "\377\377\377\377\377\377\377\377QQQ\0\0\0""444\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\375\375\377\377\377\377\374\374\377\376\376\376"
  "\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\263"
  "\263\263\0\0\0\0\0\0\262\262\262\377\377\377\376\376\376\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\376\376\376\376\376\376\377\377\377\377\377\377\321\321\321\0\0"
  "\0+++\377\377\377\375\375\375\376\376\376\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377\377"
  "\377\376\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376"
  "\376\376\376\376\377\377\377\377\377\377\371\371\371\325\325\325\377\377"
  "\377\377\377\377\377\377\377\376\376\376\377\377\377\376\376\376\376\376"
  "\376\376\376\376\376\376\376\377\377\377\376\376\376",
};



static const struct {
  guint  	 width;
  guint  	 height;
  guint  	 bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  guint8 	 pixel_data[24 * 16 * 3 + 1];
} icon_gradient = {
  24, 16, 3,
  "\376\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\10\10\10\31\32\32\40\40\40,,,777BBBMMMXXXcccoooz{{\205\205\205"
  "\220\220\220\234\234\234\247\247\247\263\263\263\275\275\275\311\310\311"
  "\324\324\324\340\340\340\352\353\353\366\366\366\377\377\377\377\377\377"
  "\10\10\10\31\31\31\40\40\40+++667BBBLLLXXXcccooo{{z\204\204\205\220\220\220"
  "\234\234\234\247\247\247\263\263\263\275\275\275\311\310\311\324\324\324"
  "\340\340\340\353\353\353\366\366\366\377\377\377\377\377\377\4\4\3\25\25"
  "\26\34\34\34(((334???JJJUUUaaammmxxx\203\203\203\216\216\216\232\233\232"
  "\246\246\246\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337"
  "\353\353\353\365\365\365\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34"
  "'((333???JJJUUUaa`mmmxxy\203\203\202\216\216\216\232\232\232\246\246\246"
  "\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337\353\352\352"
  "\365\366\366\377\377\377\377\377\377\4\3\4\25\26\25\34\34\34'((333???JJI"
  "UUUaaammmxxx\203\203\203\216\216\216\232\232\232\246\246\246\261\261\261"
  "\274\274\274\310\310\307\323\323\323\337\337\337\352\352\352\366\365\366"
  "\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34('(333???JJIUUUaaammmxxx"
  "\202\203\203\216\216\216\232\232\232\246\246\246\261\261\261\274\274\274"
  "\310\307\310\323\323\323\337\337\337\352\353\353\366\365\365\377\377\377"
  "\377\377\377\4\4\4\25\26\25\34\34\34(((433???JIJUUUaaammmxxy\203\203\202"
  "\216\216\216\233\232\232\246\246\246\261\262\261\274\274\274\307\310\310"
  "\323\323\323\337\337\337\353\352\352\365\365\365\377\377\377\377\377\377"
  "\4\4\4\26\25\25\34\34\34(((333???IIJUUUaaammmxxx\203\203\203\216\216\216"
  "\233\232\233\246\246\246\261\261\261\274\274\274\307\307\310\323\323\323"
  "\337\337\337\352\353\353\366\365\365\377\377\377\377\377\377\4\3\4\25\26"
  "\25\34\34\34(((333???IIJUUUaaammmyyy\202\202\202\216\216\216\232\232\232"
  "\246\246\246\261\261\261\274\274\274\310\310\307\323\323\323\337\337\337"
  "\353\353\352\365\365\366\377\377\377\377\377\377\3\4\4\26\25\25\34\34\34"
  "(((333???IIIUUUa`ammmxxx\203\203\203\216\216\216\232\233\232\246\246\246"
  "\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337\353\353\353"
  "\365\365\365\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34(((333???JII"
  "UUUaaammmxxy\203\203\202\216\216\216\232\232\232\246\246\246\261\261\261"
  "\274\274\274\307\310\310\323\323\323\337\337\337\353\353\352\365\365\365"
  "\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34(((333???JIIUUUaaammmxyy"
  "\203\202\203\216\216\216\232\232\232\246\246\246\261\261\261\274\274\274"
  "\310\310\310\323\323\323\337\337\337\353\352\353\365\366\365\377\377\377"
  "\377\377\377\4\4\4\26\26\26\34\34\34(((444???JJJVVVaaammmyyy\203\203\203"
  "\217\217\217\233\232\232\246\246\246\262\262\262\274\274\274\310\310\307"
  "\323\323\323\337\337\337\353\353\353\365\365\365\377\377\377\377\377\377"
  "\0\0\0\4\4\4\13\13\13\30\30\30$$$111<<<IIIUUUbbbonnzzz\206\206\206\222\223"
  "\223\237\237\237\254\254\254\267\267\267\304\304\304\320\320\320\335\335"
  "\335\351\351\351\365\365\365\377\377\377\377\377\377\252\252\252\260\260"
  "\260\262\262\262\266\266\266\272\272\272\276\276\276\301\301\301\305\306"
  "\305\312\311\311\315\315\315\321\321\321\325\325\325\331\330\330\335\335"
  "\335\340\340\340\345\344\344\350\350\350\354\354\354\360\360\360\364\364"
  "\364\370\370\370\373\373\373\377\377\377",
};


static const struct {
  guint    width;
  guint    height;
  guint    bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  guint8   pixel_data[24 * 15 * 3 + 1];
} icon_gradient_crossed = {
  24, 15, 3,
  "\207\2\2\342\2\2d\23\23%&&011;;;FFFPQP[[[fffppq{{{\206\206\205\220\220\220"
  "\233\233\233\246\245\245\260\260\260\273\273\273\305\305\306\320\321\321"
  "\333\332\332\356\227\226\376\31\31\376\206\206\322\0\0\377\0\0\377\0\0\350"
  "\5\5A--:;;FFFQQP[[[ffeppp{{{\206\206\206\220\220\220\233\233\233\245\245"
  "\245\260\260\260\273\273\273\305\306\306\325\271\271\374\23\23\377\0\0\377"
  "\0\0\377>>\30\5\5\243\7\7\377\0\0\377\0\0\377\0\0\301\23\23MCCNQQ[[[fffp"
  "pp{{{\206\206\206\220\220\220\233\233\233\246\245\245\257\262\262\277\257"
  "\257\356::\377\0\0\377\0\0\377\0\0\371ff\374\354\354\6\5\5\11\22\22P\24\24"
  "\333\6\6\377\0\0\377\0\0\377\0\0\22322]ZZeggppp{{{\206\206\206\220\220\220"
  "\232\235\235\250\241\241\320gg\377\0\0\377\0\0\377\0\0\370((\353\270\270"
  "\360\366\366\373\373\373\6\6\6\21\21\21\33\33\33$''u\40\40\374\1\1\377\0"
  "\0\377\0\0\370\4\4\203RRppp{{{\206\205\206\220\220\220\262vv\375\5\5\377"
  "\0\0\377\0\0\376\5\5\335\225\225\333\335\335\345\345\345\360\360\360\373"
  "\373\373\6\6\6\21\21\21\33\33\33&&&.12H77\246!!\377\0\0\377\0\0\377\0\0\334"
  "\34\34\207qq\223vv\346!!\377\0\0\377\0\0\377\0\0\334`a\311\273\273\320\322"
  "\322\333\333\333\345\345\345\360\360\360\374\373\373\6\6\6\21\21\21\33\33"
  "\33&&&110;;;AHHfFF\331\25\25\377\0\0\377\0\0\377\0\0\377\0\0\377\0\0\377"
  "\0\0\350**\267\240\240\271\277\277\305\305\305\320\320\320\333\333\333\345"
  "\345\345\360\360\360\373\373\373\6\6\6\21\21\21\33\33\33&&&111;;;FFFPQQX"
  "]]\25366\377\0\0\377\0\0\377\0\0\377\0\0\304]]\244\247\247\260\260\260\273"
  "\273\273\305\305\306\320\320\320\333\333\333\345\345\345\360\360\360\373"
  "\373\373\6\6\6\21\21\21\33\33\33&&&110;;;?IIu?@\341\21\21\377\0\0\377\0\0"
  "\375\2\2\377\0\0\377\0\0\377\0\0\355\"\"\276\222\222\271\301\301\306\305"
  "\305\320\320\320\333\333\333\345\345\345\360\360\360\373\373\373\6\6\6\21"
  "\21\21\33\33\33&&&-21M55\264\34\34\377\0\0\377\0\0\377\0\0\324\"\"\201vw"
  "\215~}\341''\377\0\0\377\0\0\377\0\0\342RR\312\266\265\317\323\323\333\333"
  "\333\345\345\345\360\360\360\373\373\373\6\6\6\21\21\21\33\33\33$''\213\33"
  "\33\377\0\0\377\0\0\377\0\0\357\11\11xZZqpp{{{\206\205\206\220\220\220\252"
  "\203\203\371\14\14\377\0\0\377\0\0\377\1\1\343\200\177\332\335\335\345\345"
  "\345\360\360\360\373\373\373\6\6\5\10\21\22a\22\22\343\5\5\377\0\0\377\0"
  "\0\372\2\2\21177[[[eggppq{{{\206\206\206\220\220\220\232\234\234\246\244"
  "\245\313ss\376\2\2\377\0\0\377\0\0\372\37\37\355\247\247\357\370\370\374"
  "\373\373\37\5\5\266\5\5\377\0\0\377\0\0\375\1\1\264\27\27EFFOPQ[[[fffppp"
  "{{{\206\206\205\220\220\220\233\233\233\246\245\246\257\261\261\274\270\270"
  "\353FF\377\0\0\377\0\0\377\0\0\372SR\374\346\346\330\0\0\377\0\0\377\0\0"
  "\333\7\7""7//;;;FFFQPP[[[ffeppp{{{\206\206\205\220\220\220\233\233\233\245"
  "\245\245\260\260\260\273\273\273\305\306\306\322\305\305\372\35\35\377\0"
  "\0\377\0\0\37788x\2\2\321\4\4X\24\24\"''011;;;FFFQPP[[[fffppp{{{\206\206"
  "\206\220\220\220\233\233\233\246\245\245\260\260\260\273\273\273\305\305"
  "\305\320\320\320\332\335\335\355\242\242\374++\375\224\224",
};


static const struct {
  guint    width;
  guint    height;
  guint    bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  guint8   pixel_data[24 * 15 * 3 + 1];
} icon_white = {
  24, 15, 3,
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377",
};

/*
typedef struct {
  PF::LayerTree* tree;
} TreeUpdateData;

static gboolean tree_update_cb ( TreeUpdateData* data)
{
  if( data ) {
    data->tree->update_model();
    g_free( data );
  }
  return false;
}
*/



PF::LayerTreeModel::LayerTreeModel()
{
  set_column_types( columns );
}


Glib::RefPtr<PF::LayerTreeModel> PF::LayerTreeModel::create()
{
  return Glib::RefPtr<LayerTreeModel>( new PF::LayerTreeModel() );
}


PF::LayersTreeView::LayersTreeView(PF::LayerWidget* lw): Gtk::TreeView(), layer_widget(lw)
{
  //Fill popup menu:
  auto item = Gtk::manage(new Gtk::MenuItem("_Cut", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::cut_selected_layers) );
  popupMenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Copy", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::copy_selected_layers) );
  popupMenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Paste", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::paste_layers) );
  popupMenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Delete", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::delete_selected_layers) );
  popupMenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("Load preset", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::on_button_load) );
  popupMenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("Save preset", true));
  item->signal_activate().connect(
    sigc::mem_fun(*layer_widget, &PF::LayerWidget::on_button_save) );
  popupMenu.append(*item);

  popupMenu.accelerate(*this);
  popupMenu.show_all(); //Show all menu items when the menu pops up
}


bool PF::LayersTreeView::on_button_press_event(GdkEventButton* button_event)
{
  bool return_value = false;

  //Call base class, to allow normal handling,
  //such as allowing the row to be selected by the right-click:
  //return_value = TreeView::on_button_press_event(button_event);

  //Then do our custom stuff:
  if( (button_event->type == GDK_BUTTON_PRESS) && (button_event->button == 3) ) {
    popupMenu.popup(button_event->button, button_event->time);

  } else {
    return_value = TreeView::on_button_press_event(button_event);
  }

  return return_value;

}


void PF::LayersTreeView::on_menu_cut()
{

}


void PF::LayersTreeView::on_menu_copy()
{

}


void PF::LayersTreeView::on_menu_paste()
{

}



PF::LayerTree::LayerTree( PF::ImageEditor* e, bool is_map ):
  treeView(&(e->get_layer_widget())),
  editor( e ),
  layers( NULL ),
  map_flag( is_map ),
  tree_modified(true),
  updating(false)
{
  treeModel = PF::LayerTreeModel::create();
  treeView.set_model(treeModel);
  treeView.append_column_editable("V", treeModel->columns.col_visible);
  treeView.append_column("Name", treeModel->columns.col_name);
  if( !map_flag ) {
    treeView.append_column("map1", treeModel->columns.col_omap);
    //treeView.append_column("map2", treeModel->columns.col_imap);
  }

  treeView.set_headers_visible(false);

  Gtk::TreeViewColumn* col;
  col = treeView.get_column(0);
  col->set_resizable(false); col->set_expand(true);
  //col->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
  //col->set_fixed_width(35);

  if( !map_flag ) {
    col = treeView.get_column(2);
    col->set_resizable(false); col->set_expand(false);
    //col->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
    col->set_fixed_width(30);
    /*
  col = treeView.get_column(3);
  col->set_resizable(false); col->set_expand(false);
  //col->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
  col->set_fixed_width(30);
     */
  }

  col = treeView.get_column(1);
  col->set_resizable(false); col->set_expand(true);
  //col->set_max_width(50);
  //col->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
  //col->set_fixed_width(80);


  treeView.enable_model_drag_source();
  treeView.enable_model_drag_dest();

  treeView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

  //treeView.set_size_request( 150, -1 );

  Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( treeView.get_column_cell_renderer(0) );
  cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerTree::on_cell_toggled) ); 

  treeModel->signal_dnd_done.
    connect( sigc::mem_fun(*this, &PF::LayerTree::update_model_async) );
  treeModel->signal_dnd_done.
    connect( sigc::mem_fun(*this, &PF::LayerTree::set_tree_modified) );

  signal_update_model.connect(sigc::mem_fun(*this, &LayerTree::update_model));


  add( treeView );

  set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

  //set_size_request(180,0);
  /*
  Gtk::TreeModel::Row row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer";
  row[columns.col_layer] = NULL;
  row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer 2";
  row[columns.col_layer] = NULL;
  row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer 3";
  row[columns.col_layer] = NULL;
  */
  //update_model();
}



PF::LayerTree::~LayerTree()
{
}


void PF::LayerTree::on_cell_toggled( const Glib::ustring& path )
{
  Gtk::TreeModel::iterator iter = treeView.get_model()->get_iter( path );
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    //PF::LayerTreeColumns& columns = columns;
    bool enabled = (*iter)[treeModel->columns.col_visible];
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    if( !l ) return;
#ifndef NDEBUG
    std::cout<<"Toggled visibility of layer \""<<l->get_name()<<"\": "<<enabled<<std::endl;
#endif
    if( l->get_processor() && l->get_processor()->get_par() &&
        l->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigGUI* gui =
          dynamic_cast<PF::OperationConfigGUI*>( l->get_processor()->get_par()->get_config_ui() );
      if( gui ) {
        if( enabled ) gui->show_layer();
        else gui->hide_layer();
      }
    }
    //l->set_enabled( enabled );
    //l->set_dirty( true );
    ////layer_manager->rebuild( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );
    //l->get_image()->update();
  }
}



void PF::LayerTree::update_mask_icons( Gtk::TreeModel::Row row,  PF::Layer* l )
{
  /*
  if( l->get_processor()->get_par()->has_intensity() ) {
    if( l->get_imap_layers().empty() ) {
      row[treeModel->columns.col_imap] = Gdk::Pixbuf::create_from_data(icon_white.pixel_data,Gdk::COLORSPACE_RGB,
          false, 8, icon_white.width, icon_white.height, icon_white.width*3);
    } else {
      if( l->get_processor()->get_par()->get_mask_enabled() ) {
        row[treeModel->columns.col_imap] = Gdk::Pixbuf::create_from_data(icon_gradient.pixel_data,Gdk::COLORSPACE_RGB,
            false, 8, icon_gradient.width, icon_gradient.height, icon_gradient.width*3);
      } else {
        row[treeModel->columns.col_imap] = Gdk::Pixbuf::create_from_data(icon_gradient_crossed.pixel_data,Gdk::COLORSPACE_RGB,
            false, 8, icon_gradient_crossed.width, icon_gradient_crossed.height, icon_gradient_crossed.width*3);
      }
    }
  }
  */
  if( !l ) {
    std::cout<<"LayerTree::update_mask_icons(): l==NULL"<<std::endl;
    return;
  }
  if( !l->get_processor() ) {
    std::cout<<"LayerTree::update_mask_icons(): layer \""<<l->get_name()<<"\" l->get_processor()==NULL"<<std::endl;
    return;
  }
  if( !l->get_processor()->get_par() ) {
    std::cout<<"LayerTree::update_mask_icons(): layer \""<<l->get_name()<<"\" l->get_processor()->get_par()==NULL"<<std::endl;
    return;
  }
  if( l->get_processor()->get_par()->has_opacity() ) {
    if( l->get_omap_layers().empty() ) {
      row[treeModel->columns.col_omap] = Gdk::Pixbuf::create_from_data(icon_white.pixel_data,Gdk::COLORSPACE_RGB,
          false, 8, icon_white.width, icon_white.height, icon_white.width*3);
    } else {
      if( l->get_blender()->get_par()->get_mask_enabled() ) {
        row[treeModel->columns.col_omap] = Gdk::Pixbuf::create_from_data(icon_gradient.pixel_data,Gdk::COLORSPACE_RGB,
            false, 8, icon_gradient.width, icon_gradient.height, icon_gradient.width*3);
      } else {
        row[treeModel->columns.col_omap] = Gdk::Pixbuf::create_from_data(icon_gradient_crossed.pixel_data,Gdk::COLORSPACE_RGB,
            false, 8, icon_gradient_crossed.width, icon_gradient_crossed.height, icon_gradient_crossed.width*3);
      }
    }
  }
}



void PF::LayerTree::update_model_int( Gtk::TreeModel::Row parent_row )
{
  //PF::LayerTreeColumns& columns = columns;
  PF::Layer* parent_layer = parent_row[treeModel->columns.col_layer];
  if( !parent_layer ) return;

  Gtk::TreeModel::Row row = *(treeModel->append(parent_row.children()));
  row[treeModel->columns.col_visible] = false;
  row[treeModel->columns.col_name] = std::string( "" );
  row[treeModel->columns.col_layer] = NULL;

  std::list<Layer*> sublayers = parent_layer->get_sublayers();
  for( std::list<Layer*>::iterator li = sublayers.begin();
       li != sublayers.end(); li++ ) {
    PF::Layer* l = *li;
    Gtk::TreeModel::iterator iter = treeModel->prepend(parent_row.children());
    row = *(iter);
    row[treeModel->columns.col_visible] = l->is_enabled();
    row[treeModel->columns.col_name] = l->get_name().substr(0,15);
    row[treeModel->columns.col_layer] = l;
    update_mask_icons( row, l );

    if( l->get_processor() && l->get_processor()->get_par() ) {
      PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( l->get_processor()->get_par()->get_config_ui() );
      if( ui ) ui->set_editor( editor );
    }

    if( l->is_group() ) {
      update_model_int( row );
      Gtk::TreeModel::Path path = treeModel->get_path( iter );
      if( l->is_expanded() ) {
        treeView.expand_row( path, true );
      } else {
        treeView.collapse_row( path );
      }
    }
  }
}



void PF::LayerTree::update_model_async()
{
  signal_update_model.emit();
}


void PF::LayerTree::update_model()
{
#ifndef NDEBUG
  std::cout<<"LayerTree::update_model(): get_tree_modified()="<<get_tree_modified()<<std::endl;
#endif
  if( get_tree_modified() == false )
    return;

  if( updating ) return;

  tree_modified = false;
  updating = true;

#ifndef NDEBUG
  std::cout<<"LayerTree::update_model(): treeModel->clear() called."<<std::endl;
#endif
  treeModel->clear();
#ifndef NDEBUG
  std::cout<<"LayerTree::update_model(): after treeModel->clear()"<<std::endl;
#endif

  if( layers ) {

#ifndef NDEBUG
    std::cout<<"LayerTree::update_model(): layers->size()="<<layers->size()<<""<<std::endl;
#endif
    std::list<PF::Layer*>::iterator li;
    for( li = layers->begin(); li != layers->end(); li++ ) {
      PF::Layer* l = *li;
      if( !l ) continue;
      if( !l->get_processor() ) {
        std::cout<<"LayerTree::update_model(): NULL processor for layer \""<<l->get_name()<<"\""<<std::endl;
        continue;
      }
      if( !l->get_processor()->get_par() ) {
        std::cout<<"LayerTree::update_model(): NULL operation for layer \""<<l->get_name()<<"\""<<std::endl;
        continue;
      }
#ifndef NDEBUG
      std::cout<<"LayerTree::update_model(): adding layer \""<<l->get_name()<<"\""<<std::endl;
#endif
      Gtk::TreeModel::iterator iter = treeModel->prepend();
      Gtk::TreeModel::Row row = *(iter);
      row[treeModel->columns.col_visible] = l->is_enabled();
      row[treeModel->columns.col_name] = l->get_name().substr(0,15);
      row[treeModel->columns.col_layer] = l;
      update_mask_icons( row, l );

      if( l->get_processor() && l->get_processor()->get_par() ) {
        PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( l->get_processor()->get_par()->get_config_ui() );
        if( ui ) ui->set_editor( editor );
      }

      if( l->is_group() ) {
        update_model_int( row );
        Gtk::TreeModel::Path path = treeModel->get_path( iter );
        if( l->is_expanded() ) {
          //std::cout<<"LayerTree::update_model(): expanding row"<<std::endl;
          treeView.expand_row( path, true );
        } else {
          //std::cout<<"LayerTree::update_model(): collapsing row"<<std::endl;
          treeView.collapse_row( path );
        }
      }
    }
  }
  //treeView.expand_all();
  treeView.columns_autosize();

  signal_updated.emit();

  updating = false;

  //std::cout<<"LayerTree::update_model() finished"<<std::endl;
/*
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      get_tree().get_selection();
  if( refTreeSelection->count_selected_rows() == 0 ) {
    Gtk::TreeModel::Children children = get_model()->children();
    refTreeSelection->select( children.begin() );
  }
*/

  /*
  if (!image) {
    treeModel->clear();
    return;
  }

  const std::vector<PF::Layer*>& layers = image->get_layers();
  Gtk::TreeModel::Children children = treeModel->children();
  Gtk::TreeModel::iterator iter;
  int layerid;
  for (iter=children.begin(), layerid=0; iter != children.end(); iter++, layerid++) {
    if (layerid >= layers.size()) break;
    bool visible = layers[layerid]->is_enabled();
    const std::string& name = layers[layerid]->get_name();
    (*iter)[columns.col_visible] = visible;
    (*iter)[columns.col_name] = name;
    (*iter)[columns.col_layer] = layers[layerid];
  }

  if ( layerid >= layers.size() && iter != children.end() ) {
    // clear list items that do not correspond anymore to layers
    for( ; iter != children.end();) {
      iter = treeModel->erase(iter);
    }
  }

  if (layerid < layers.size()) {
    // Append additional layers at the end of the list
    for (; layerid < layers.size(); layerid++) {
      bool visible = layers[layerid]->is_enabled();
      const std::string& name = layers[layerid]->get_name();
      Gtk::TreeModel::Row row = *(treeModel->append());
      row[columns.col_visible] = visible;
      row[columns.col_name] = name;
      row[columns.col_layer] = layers[layerid];
    }
  }
  */
}



PF::Layer* PF::LayerTree::get_selected_layer()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    iter = get_model()->get_iter( sel_rows[0] );
  }
  //Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    return l;
  }
  return NULL;
}



int PF::LayerTree::get_selected_layer_id()
{
  int result = -1;
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    iter = get_model()->get_iter( sel_rows[0] );
  }
  //Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    if(l) result = l->get_id();
  }

  return( result );
}



bool PF::LayerTree::get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter)
{
#ifndef NDEBUG
  std::cout<<"[LayerTree::get_row] looking for layer id "<<id<<std::endl;
#endif
  for(  Gtk::TreeModel::iterator it = rows.begin();
        it != rows.end(); it++ ) {
    //Gtk::TreeModel::Row row = *it;
#ifndef NDEBUG
    std::cout<<"[LayerTree::get_row] getting layer of row \""<<(*it)[treeModel->columns.col_name]<<"\"\n";
#endif
    PF::Layer* l = (*it)[treeModel->columns.col_layer];
    if(l && ((int)(l->get_id())==id)) {
      iter = it;
      return true;
    }
    Gtk::TreeModel::Children children = it->children();
    if( !children.empty() ) {
      if( get_row( id, children, iter ) )
        return true;
    }
  }
  return false;
}



bool PF::LayerTree::get_row(int id, Gtk::TreeModel::iterator& iter)
{
  Glib::RefPtr<Gtk::TreeStore> model = get_model();
  const Gtk::TreeModel::Children rows = model->children();
  return get_row( id, rows, iter );
}


void PF::LayerTree::select_row(int id)
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  Gtk::TreeModel::iterator iter;
  if( get_row( id, iter ) ) {
    refTreeSelection->select( iter );
  }
}



