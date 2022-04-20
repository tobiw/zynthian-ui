# -*- coding: utf-8 -*-
#******************************************************************************
# ZYNTHIAN PROJECT: Zynthian Chain (zynthian_chain)
#
# a chain is composed of a graph of connected nodes (no loops!)
# + the node graph is represented by a grid of NxM, so nodes in the same column are connected in parallel, etc.
# + it can listen to 0 or more midi_chans, re-mapping the channels as defined
# + it can listen to 0 or more MIDI devices (ZMIP_DEVs) or ports
# + it can send MIDI to 0 or more MIDI devices/ports
# + it's connected to a list of audio inputs
# + it's connected to a list of audio outputs, including a zynmixer strip
#
# Copyright (C) 2015-2022 Fernando Moyano <jofemodo@zynthian.org>
#
#******************************************************************************
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
#******************************************************************************

import os
import copy
import logging
from collections import OrderedDict

# Zynthian specific modules
from zyncoder.zyncore import lib_zyncore

#------------------------------------------------------------------------------

COLS_NODE_GRID = 16
ROWS_NODE_GRID = 16

class zynthian_chain:

	# ---------------------------------------------------------------------------
	# Initialization
	# ---------------------------------------------------------------------------

	def __init__(self):
		# Node graph is represented by a grid.
		# Nodes in the same column are connected in parallel.
		# etc.
		self.nodes = [[None for col in range(COLS_NODE_GRID)] for row in range(ROWS_NODE_GRID)]

		# A list of device strings, as returned by autoconnect's get_port_alias_id() function.
		# Also, see zynthian_autoconnect#224 => devices_in
		self.midi_input_devs = []
		self.midi_output_devs = []

		# MIDI input chans are represented by a map.
		# For instance, set "self.midi_input_chans[5]=5" to listen channel 5 without mapping.
		# For instance, set "self.midi_input_chans[5]=6" to listen channel 5 and map it to channel 6.
		# For instance, set "self.midi_input_chans[5]=None" to ignore channel 5 events.
		self.midi_input_chans = [None for chan in range(16)]

		# Note range & transposing
		self.note_range_trans_config = {
			'note_low': 0,
			'note_high': 127,
			'octave_trans': 0,
			'halftone_trans': 0 
		}

		# A list of audio port names
		self.audio_input = []

		# A list of audio port names
		self.audio_output = []

		# If no title is set, a run-time title is generated using the root node + preset name
		self.title = None


	def reset(self):
		pass


	# ---------------------------------------------------------------------------
	# MIDI Input
	# ---------------------------------------------------------------------------

	def set_midi_input_devs(self, midi_devs):
		self.midi_input_devs = midi_devs


	def set_midi_input_chans(self, midi_chans):
		self.midi_input_chans = midi_chans


	# ---------------------------------------------------------------------------
	# Snapshot Management
	# ---------------------------------------------------------------------------

	def get_snapshot(self):
		pass


	def restore_snapshot(self, snapshot):
		pass

	# ---------------------------------------------------------------------------
	# ZS3 Management (Zynthian SubSnapShots)
	# ---------------------------------------------------------------------------

	def reset_zs3(self):
		self.zs3_list = [None]*128


	def delete_zs3(self, i):
		self.zs3_list[i] = None


	def get_zs3(self, i):
		return self.zs3_list[i]


	def save_zs3(self, i):
		try:
			zs3 = { 
				#TODO
			}
			self.zs3_list[i] = zs3

		except Exception as e:
			logging.error(e)


	def restore_zs3(self, i):
		zs3 = self.zs3_list[i]
		if zs3:
			#TODO
			return True
		else:
			return False


	# ---------------------------------------------------------------------------
	# Audio Output Routing:
	# ---------------------------------------------------------------------------


	def get_jackname(self):
		return self.jackname


	def get_audio_jackname(self):
		return self.jackname


	def get_audio_out(self):
		return self.audio_out


	def set_audio_out(self, ao):
		self.audio_out = copy.copy(ao)

		#Fix legacy routing (backward compatibility with old snapshots)
		if "system" in self.audio_out:
			self.audio_out.remove("system")
			self.audio_out += ["system:playback_1", "system:playback_2"]

		self.pair_audio_out()
		self.zyngui.zynautoconnect_audio()


	def add_audio_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_audio_jackname()

		if jackname not in self.audio_out:
			self.audio_out.append(jackname)
			logging.debug("Connecting Audio Output {} => {}".format(self.get_audio_jackname(), jackname))

		self.pair_audio_out()
		self.zyngui.zynautoconnect_audio()


	def del_audio_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_audio_jackname()

		try:
			self.audio_out.remove(jackname)
			logging.debug("Disconnecting Audio Output {} => {}".format(self.get_audio_jackname(), jackname))
		except:
			pass

		self.pair_audio_out()
		self.zyngui.zynautoconnect_audio()


	def toggle_audio_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_audio_jackname()

		if jackname not in self.audio_out:
			self.audio_out.append(jackname)
		else:
			self.audio_out.remove(jackname)

		self.pair_audio_out()
		self.zyngui.zynautoconnect_audio()


	def reset_audio_out(self):
		self.audio_out=["system:playback_1", "system:playback_2"]
		if self.midi_chan != None:
			self.audio_out = ["zynmixer:input_%02da"%(self.midi_chan + 1), "zynmixer:input_%02db"%(self.midi_chan + 1)]
		#self.pair_audio_out() #TODO: This was previously removed - is it required?
		self.zyngui.zynautoconnect_audio()


	def mute_audio_out(self):
		self.audio_out = []
		self.pair_audio_out()
		self.zyngui.zynautoconnect_audio()


	def pair_audio_out(self):
		if not self.engine.options['layer_audio_out']:
			for l in self.engine.layers:
				if l!=self:
					l.audio_out = self.audio_out
					#logging.debug("Pairing CH#{} => {}".format(l.midi_chan,l.audio_out))


	# ---------------------------------------------------------------------------
	# Audio Input Routing:
	# ---------------------------------------------------------------------------


	def get_audio_in(self):
		return self.audio_in


	def set_audio_in(self, ai):
		self.audio_in = copy.copy(ai)
		self.zyngui.zynautoconnect_audio()


	def add_audio_in(self, jackname):
		if jackname not in self.audio_in:
			self.audio_in.append(jackname)
			logging.debug("Connecting Audio Capture {} => {}".format(jackname, self.get_audio_jackname()))

		self.zyngui.zynautoconnect_audio()


	def del_audio_in(self, jackname):
		try:
			self.audio_in.remove(jackname)
			logging.debug("Disconnecting Audio Capture {} => {}".format(jackname, self.get_audio_jackname()))
		except:
			pass

		self.zyngui.zynautoconnect_audio()


	def toggle_audio_in(self, jackname):
		if jackname not in self.audio_in:
			self.audio_in.append(jackname)
		else:
			self.audio_in.remove(jackname)

		logging.debug("Toggling Audio Capture: {}".format(jackname))

		self.zyngui.zynautoconnect_audio()


	def reset_audio_in(self):
		self.audio_in=["system:capture_1", "system:capture_2"]
		self.zyngui.zynautoconnect_audio()


	def mute_audio_in(self):
		self.audio_in=[]
		self.zyngui.zynautoconnect_audio()


	def is_parallel_audio_routed(self, layer):
		if isinstance(layer, zynthian_layer) and layer!=self and layer.midi_chan==self.midi_chan and collections.Counter(layer.audio_out)==collections.Counter(self.audio_out):
			return True
		else:
			return False

	# ---------------------------------------------------------------------------
	# MIDI Routing:
	# ---------------------------------------------------------------------------

	def get_midi_jackname(self):
		return self.engine.jackname


	def get_midi_out(self):
		return self.midi_out


	def set_midi_out(self, mo):
		self.midi_out=mo
		#logging.debug("Setting MIDI connections:")
		#for jn in mo:
		#	logging.debug("  {} => {}".format(self.engine.jackname, jn))
		self.zyngui.zynautoconnect_midi()


	def add_midi_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_midi_jackname()

		if jackname not in self.midi_out:
			self.midi_out.append(jackname)
			logging.debug("Connecting MIDI {} => {}".format(self.get_midi_jackname(), jackname))

		self.zyngui.zynautoconnect_midi()


	def del_midi_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_midi_jackname()

		try:
			self.midi_out.remove(jackname)
			logging.debug("Disconnecting MIDI {} => {}".format(self.get_midi_jackname(), jackname))
		except:
			pass

		self.zyngui.zynautoconnect_midi()


	def toggle_midi_out(self, jackname):
		if isinstance(jackname, zynthian_layer):
			jackname=jackname.get_midi_jackname()

		if jackname not in self.midi_out:
			self.midi_out.append(jackname)
		else:
			self.midi_out.remove(jackname)

		self.zyngui.zynautoconnect_midi()


	def mute_midi_out(self):
		self.midi_out=[]
		self.zyngui.zynautoconnect_midi()


	def is_parallel_midi_routed(self, layer):
		if isinstance(layer, zynthian_layer) and layer!=self and layer.midi_chan==self.midi_chan and collections.Counter(layer.midi_out)==collections.Counter(self.midi_out):
			return True
		else:
			return False


	# ---------------------------------------------------------------------------
	# Channel "Path" String
	# ---------------------------------------------------------------------------


	def get_path(self):
		path = self.bank_name
		if self.preset_name:
			path = path + "/" + self.preset_name
		return path


	def get_basepath(self):
		path = self.engine.get_path(self)
		if self.midi_chan is not None:
			path = "{}#{}".format(self.midi_chan+1, path)
		return path


	def get_bankpath(self):
		path = self.get_basepath()
		if self.bank_name and self.bank_name!="None":
			path += " > " + self.bank_name
		return path


	def get_presetpath(self):
		path = self.get_basepath()

		subpath = None
		if self.bank_name and self.bank_name!="None":
			subpath = self.bank_name
			if self.preset_name:
				subpath += "/" + self.preset_name
		elif self.preset_name:
			subpath = self.preset_name

		if subpath:
			path += " > " + subpath

		return path


#******************************************************************************
