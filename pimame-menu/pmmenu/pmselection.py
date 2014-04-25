import os
import pygame


class PMSelection(pygame.sprite.Sprite):
	def __init__(self, global_opts):
		pygame.sprite.Sprite.__init__(self)

		#screen_width = pygame.display.Info().current_w
		#item_width = ((screen_width - global_opts.padding) / global_opts.num_items_per_row) - global_opts.padding
		#item_height = global_opts.item_height

		#colorkey_color = (0, 0, 0)
		#if colorkey_color == global_opts.selection_color:
			#colorkey_color = (255, 255, 255)

		#self.image = pygame.Surface([item_width, global_opts.item_height])
		#self.image.fill(colorkey_color)
		#self.image.set_colorkey(colorkey_color)
		#pygame.draw.rect(self.image, global_opts.selection_color, (0, 0, item_width, global_opts.item_height), global_opts.selection_size)
		#pygame.draw.lines(self.image, global_opts.selection_color, True, [(0, 0), (item_width, 0), (item_width, item_height), (0, item_height)], global_opts.selection_size)

		#self.rect = self.image.get_rect()

	def update(self, menu_item, global_opts):
		#pygame.sprite.Sprite.__init__(self)

		screen_width = pygame.display.Info().current_w
		item_width = ((screen_width - global_opts.padding) / global_opts.num_items_per_row) - global_opts.padding

		self.image = pygame.Surface([item_width, global_opts.item_height], pygame.SRCALPHA, 32).convert_alpha()
		#self.image = image.convert_alpha()
		#self.image.fill((0,0,0,0))
		
		item_rect = menu_item.rect
		if menu_item.icon_selected:
			#icon_file_path = menu_item.icon_selected
			icon = menu_item.pre_loaded_selected_icon
			
			# resize and center icon:
			icon_size = icon.get_size()
			avail_icon_width = item_width - global_opts.padding * 2
			avail_icon_height = global_opts.item_height - global_opts.padding * 2
			while True:
				icon_width = icon_size[0]
				icon_height = icon_size[1]
				icon_ratio = float(icon_height) / float(icon_width)
				icon_width_diff = avail_icon_width - icon_width
				icon_height_diff = avail_icon_height - icon_height
				if icon_width_diff < icon_height_diff:
					diff = icon_width_diff
					icon_size = (icon_width + diff, icon_height + diff * icon_ratio)
				else:
					diff = icon_height_diff
					icon_size = (icon_width + diff / icon_ratio, icon_height + diff)

				icon_size = (int(icon_size[0]), int(icon_size[1]))

				if icon_size[0] <= avail_icon_width and icon_size[1] <= avail_icon_height:
					break

			icon = pygame.transform.scale(icon, icon_size)
			self.image.blit(icon, ((avail_icon_width - icon_size[0]) / 2 + global_opts.padding, (avail_icon_height - icon_size[1]) / 2 + global_opts.padding))
			
			self.rect = self.image.get_rect()

		self.rect.x = item_rect.x;
		self.rect.y = item_rect.y;
