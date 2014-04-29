import os
import pygame
from pmlabel import *


class PMList(pygame.sprite.OrderedUpdates):
	labels = []

	def __init__(self, rom_list, global_opts):
		pygame.sprite.OrderedUpdates.__init__(self)

		self.first_index = self.last_index = 0
		self.labels = []

		self.rom_list = sorted(rom_list, key=lambda rom: rom['title'])

		back_item = {'type': 'back', 'title': '<- Back', 'command': None}
		self.rom_list.insert(0, back_item)
		
		#get pre-loaded (unselected) rom list image
		create_romlist_image = global_opts.pre_loaded_romlist.convert_alpha()
		#Create rom list surface/image with no text
		rom_template = PMLabel('', global_opts.font, global_opts.text_color, global_opts.background_color, global_opts.label_padding, create_romlist_image)

		#Get rom title and blit to already created rom_template
		for list_item in self.rom_list:
			label = PMLabel(list_item['title'], global_opts.font, global_opts.text_color, global_opts.background_color, global_opts.label_padding, False, rom_template)
			label.type = list_item['type']
			label.command = list_item['command']
			self.labels.append(label)

	def set_visible_items(self, first_index, last_index):
		self.first_index = first_index
		self.last_index = last_index

		self.empty()
		self.add(self.labels[first_index:last_index])