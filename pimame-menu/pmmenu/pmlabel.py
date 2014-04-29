import os
import pygame


class PMLabel(pygame.sprite.Sprite):
	def __init__(self, label_text, font, color_fg, color_bg, label_padding, create_romlist_image = False, new_rom = False):
		pygame.sprite.Sprite.__init__(self)

		
		self.text = label_text
		self.color_fg = color_fg
		self.font = font
		
		#font_opts = font_file, font_size, color_fg, color_bg
		
		if create_romlist_image or new_rom:
			if new_rom:
				self.image = new_rom.image.copy()
				text = font.render(label_text, 1, color_fg)
				text_rect = text.get_rect()
				text_rect =  (label_padding['left'] , ((new_rom.icon_rect.h - text_rect.h) / 2) + label_padding['top'])
				area_rect = [0,0,new_rom.icon_rect.w - label_padding['right'] - label_padding['left'],new_rom.icon_rect.h - label_padding['bottom']]
				self.image.blit(text, text_rect, area_rect)
			else:
				self.icon = create_romlist_image
				self.icon_rect = self.icon.get_rect()
				self.image = pygame.Surface([self.icon_rect.w,self.icon_rect.h], pygame.SRCALPHA, 32).convert_alpha()
				self.image.blit(self.icon, (0,0))

		else:
			text = font.render(label_text, 1, color_fg, color_bg)
			text_rect = text.get_rect()
			self.image = pygame.Surface([text_rect.w, text_rect.h], pygame.SRCALPHA, 32).convert_alpha()
			self.image.blit(text, text_rect)
		
		
		
		#colorkey_color = (255, 255, 255)

		
		#self.image.fill(colorkey_color)
		#self.image.set_colorkey(colorkey_color)
		
		self.rect = self.image.get_rect()
		