#!/usr/bin/python3
# -*- coding: utf-8 -*-
#********************************************************************
# ZYNTHIAN PROJECT: Zynmidicontroller Python Wrapper
#
# A Python wrapper for zynmidicontroller library
#
# Copyright (C) 2021 Brian Walton <brian@riban.co.uk>
#
#********************************************************************
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# For a full copy of the GNU General Public License see the LICENSE.txt file.
#
#********************************************************************

import ctypes
from _ctypes import dlclose
from os.path import dirname, realpath

libcontroller = None

#-------------------------------------------------------------------------------
# 	Zynthian MIDI Controller Library Wrapper
#
#	Most library functions are accessible directly by calling libmidicontroller.functionName(parameters)
#	Following function wrappers provide simple access for complex data types. Access with zynmidicontroller.function_name(parameters)
#
#	Include the following imports to access these two library objects:
# 		from zynlibs.zynmidicontroller import zynmidicontroller
#		from zynlibs.zynmidicontroller.zynmidicontroller import libmidicontroller
#
#-------------------------------------------------------------------------------

#	Initiate library - performed by zynmidicontroller module
def init():
	global libcontroller
	if libcontroller:
	 	return
	try:
		libcontroller = ctypes.cdll.LoadLibrary(dirname(realpath(__file__))+"/build/libzynmidicontroller.so")
		libcontroller.getSupported.restype = ctypes.c_char_p
	except Exception as e:
		libcontroller=None
		print("Can't initialise zynmidicontroller library: %s" % str(e))


#	Destoy instance of shared library
def destroy():
	global libcontroller
	if libcontroller:
		dlclose(libcontroller._handle)
	libcontroller = None


#	Get supported interface
#	rest: True to reset to start of list
#	Returns: Name of MIDI port alias for next supported device or None if end of list
#	If a supported device is already connected then only this alias will be returned
def get_supported(reset):
	result = libcontroller.getSupported(reset)
	if result:
		return result.decode("utf-8")
	return None


#-------------------------------------------------------------------------------
