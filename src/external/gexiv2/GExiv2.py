# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab
#
# Copyright (C) 2012 Robert Park <r@robru.ca>
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

from datetime import datetime
from fractions import Fraction

from gi.repository import GObject
from ..overrides import override
from ..module import get_introspection_module

GExiv2 = get_introspection_module('GExiv2')

__all__ = []

DATE_FORMAT = '%Y:%m:%d %H:%M:%S'
MAX_INT = 2**31 - 1

@override
class Metadata(GExiv2.Metadata):
    def __init__(self, path=None):
        super(Metadata, self).__init__()
        self._path = path
        if path is not None:
            self.open_path(path)
    
    def open_path(self, path):
        self._path = path
        super(Metadata, self).open_path(path)
    
    def save_file(self, path=None):
        super(Metadata, self).save_file(path or self._path)
    
    def get_date_time(self):
        datestring = self['Exif.Photo.DateTimeOriginal']
        if datestring is not None:
            return datetime.strptime(datestring, DATE_FORMAT)
    
    def set_date_time(self, value):
        self['Exif.Photo.DateTimeOriginal'] = value.strftime(DATE_FORMAT)
    
    def get_exposure_time(self):
        num, denom = super(Metadata, self).get_exposure_time()
        return Fraction(num, denom) if denom else None
    
    def get_exif_tag_rational(self, key):
        num, denom = super(Metadata, self).get_exif_tag_rational(key)
        return Fraction(num, denom) if denom else None
    
    def set_exif_tag_rational(self, key, fraction):
        limit = MAX_INT
        while True:
            try:
                return super(Metadata, self).set_exif_tag_rational(
                    key, fraction.numerator, fraction.denominator)
            except ValueError:
                fraction = fraction.limit_denominator(limit)
                limit = int(limit / 2)
    
    def get_tags(self):
        return self.get_exif_tags() + self.get_iptc_tags() + self.get_xmp_tags()
    
    def get(self, key, default=None):
        return self.get_tag_string(key) if self.has_tag(key) else default

    def get_raw(self, key):
        return self.get_tag_raw(key).get_data()
    
    def __iter__(self):
        return iter(self.get_tags())
    
    def __contains__(self, key):
        return self.has_tag(key)
    
    def __len__(self):
        return len(self.get_tags())
    
    def __getitem__(self, key):
        if self.has_tag(key):
            return self.get_tag_string(key)
        else:
            raise KeyError('%s: Unknown tag' % key)
    
    def __delitem__(self, key):
        if self.has_tag(key):
            self.clear_tag(key)
        else:
            raise KeyError('%s: Unknown tag' % key)
    
    __setitem__ = GExiv2.Metadata.set_tag_string

__all__.append('Metadata')

if not GExiv2.initialize():
    raise RuntimeError("GExiv2 couldn't be initialized")

