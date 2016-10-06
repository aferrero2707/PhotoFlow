# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab
#
# Copyright (C) 2014 Alexandre Rossi <alexandre.rossi@gmail.com>
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA


import unittest
import os


# Workaround pkutil.extend_path in dist-packages/gi/overrides/__init__.py
# not helping us here for obscure reasons when using python2 (the following
# is useless in python3).
import gi.overrides
gi.overrides.__path__.append(os.path.join(os.path.dirname(__file__),
                                          'gi', 'overrides'))


from gi.repository import GExiv2


SAMPLES_DIR = os.path.join(os.path.dirname(__file__), '..')


class TestGexiv2(unittest.TestCase):

    def get_sample_path(self, sample):
        return os.path.join(SAMPLES_DIR, sample)

    def test_author_badencoding(self):
        sample = 'sample-author-badencoding.jpg'
        md = GExiv2.Metadata(self.get_sample_path(sample))

        self.assertEqual(md.get_raw('Exif.Image.Artist'), b'\xc0\xeb\xe5\xea\xf1\xe0\xed\xe4\xf0 \xca\xee\xf8\xe5\xeb\xe5\xe2\x00')


if __name__ == '__main__':
    unittest.main()
