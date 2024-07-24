#include "c.h"

static usize hashtable_size = 512;
static TokenData lbrace = {
	.string = "{",
	.token = {
		.type = TOKEN_LBRACE,
		.value = 0,
	}
};
static TokenData rbrace = {
	.string = "}",
	.token = {
		.type = TOKEN_RBRACE,
		.value = 0,
	}
};
static TokenData plus = {
	.string = "+",
	.token = {
		.type = TOKEN_PLUS,
		.value = 0,
	}
};
static TokenData equal = {
	.string = "=",
	.token = {
		.type = TOKEN_EQUAL,
		.value = 0,
	}
};
static TokenData tk_spawn = {
	.string = "spawn",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_SPAWN,
	}
};
static TokenData tk_reload = {
	.string = "reload",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_RELOAD,
	}
};
static TokenData tk_quit = {
	.string = "quit",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_QUIT,
	}
};
static TokenData tk_kill = {
	.string = "kill",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_KILL,
	}
};
static TokenData tk_focus_next = {
	.string = "focus_next",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_FOCUS_NEXT,
	}
};
static TokenData tk_focus_prev = {
	.string = "focus_prev",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_FOCUS_PREV,
	}
};
static TokenData tk_reveal_all = {
	.string = "reveal_all",
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_REVEAL_ALL,
	}
};
static TokenData mod_alt = {
	.string = "alt",
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 1,
	}
};
static TokenData mod_ctrl = {
	.string = "ctrl",
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 2,
	}
};
static TokenData mod_shift = {
	.string = "shift",
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 4,
	}
};
static TokenData mod_win = {
	.string = "win",
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 8,
	}
};
static TokenData attr_key = {
	.string = "key",
	.token = {
		.type = TOKEN_ACTION_ATTRIBUTE,
		.value = ATTRIBUTE_KEY,
	}
};
static TokenData attr_modifier = {
	.string = "modifier",
	.token = {
		.type = TOKEN_ACTION_ATTRIBUTE,
		.value = ATTRIBUTE_MODIFIER,
	}
};
static TokenData attr_arg = {
	.string = "arg",
	.token = {
		.type = TOKEN_ACTION_ATTRIBUTE,
		.value = ATTRIBUTE_ARG,
	}
};
static TokenData key_a = {
	.string = "a",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x41,
	}
};
static TokenData key_b = {
	.string = "b",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x42,
	}
};
static TokenData key_c = {
	.string = "c",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x43,
	}
};
static TokenData key_d = {
	.string = "d",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x44,
	}
};
static TokenData key_e = {
	.string = "e",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x45,
	}
};
static TokenData key_f = {
	.string = "f",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x46,
	}
};
static TokenData key_g = {
	.string = "g",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x47,
	}
};
static TokenData key_h = {
	.string = "h",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x48,
	}
};
static TokenData key_i = {
	.string = "i",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x49,
	}
};
static TokenData key_j = {
	.string = "j",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4a,
	}
};
static TokenData key_k = {
	.string = "k",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4b,
	}
};
static TokenData key_l = {
	.string = "l",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4c,
	}
};
static TokenData key_m = {
	.string = "m",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4d,
	}
};
static TokenData key_n = {
	.string = "n",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4e,
	}
};
static TokenData key_o = {
	.string = "o",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4f,
	}
};
static TokenData key_p = {
	.string = "p",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x50,
	}
};
static TokenData key_q = {
	.string = "q",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x51,
	}
};
static TokenData key_r = {
	.string = "r",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x52,
	}
};
static TokenData key_s = {
	.string = "s",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x53,
	}
};
static TokenData key_t = {
	.string = "t",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x54,
	}
};
static TokenData key_u = {
	.string = "u",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x55,
	}
};
static TokenData key_v = {
	.string = "v",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x56,
	}
};
static TokenData key_w = {
	.string = "w",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x57,
	}
};
static TokenData key_x = {
	.string = "x",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x58,
	}
};
static TokenData key_y = {
	.string = "y",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x59,
	}
};
static TokenData key_z = {
	.string = "z",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x5a,
	}
};
static TokenData key_f1 = {
	.string = "f1",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x70,
	}
};
static TokenData key_f2 = {
	.string = "f2",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x71,
	}
};
static TokenData key_f3 = {
	.string = "f3",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x72,
	}
};
static TokenData key_f4 = {
	.string = "f4",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x73,
	}
};
static TokenData key_f5 = {
	.string = "f5",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x74,
	}
};
static TokenData key_f6 = {
	.string = "f6",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x75,
	}
};
static TokenData key_f7 = {
	.string = "f7",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x76,
	}
};
static TokenData key_f8 = {
	.string = "f8",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x77,
	}
};
static TokenData key_f9 = {
	.string = "f9",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x78,
	}
};
static TokenData key_f10 = {
	.string = "f10",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x79,
	}
};
static TokenData key_f11 = {
	.string = "f11",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7a,
	}
};
static TokenData key_f12 = {
	.string = "f12",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7b,
	}
};
static TokenData key_f13 = {
	.string = "f13",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7c,
	}
};
static TokenData key_f14 = {
	.string = "f14",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7d,
	}
};
static TokenData key_f15 = {
	.string = "f15",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7e,
	}
};
static TokenData key_f16 = {
	.string = "f16",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7f,
	}
};
static TokenData key_f17 = {
	.string = "f17",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x80,
	}
};
static TokenData key_f18 = {
	.string = "f18",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x81,
	}
};
static TokenData key_f19 = {
	.string = "f19",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x82,
	}
};
static TokenData key_f20 = {
	.string = "f20",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x83,
	}
};
static TokenData key_f21 = {
	.string = "f21",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x84,
	}
};
static TokenData key_f22 = {
	.string = "f22",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x85,
	}
};
static TokenData key_f23 = {
	.string = "f23",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x86,
	}
};
static TokenData key_f24 = {
	.string = "f24",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x87,
	}
};
static TokenData enter = {
	.string = "enter",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x0D,
	}
};
static TokenData space = {
	.string = "space",
	.token = {
		.type = TOKEN_KEY,
		.value = 0x20,
	}
};
static TokenData desktops_tok = {
	.string = "desktops",
	.token = {
		.type = TOKEN_DESKTOPS,
		.value = 0,
	}
};
static TokenData send_to_desktop_tok = {
	.string = "send_to_desktop_modifier",
	.token = {
		.type = TOKEN_DESKTOPS_ATTRIBUTE,
		.value = ATTRIBUTE_SEND_TO_DESKTOP,
	}
};
static TokenData switch_to_desktop_tok = {
	.string = "switch_to_desktop_modifier",
	.token = {
		.type = TOKEN_DESKTOPS_ATTRIBUTE,
		.value = ATTRIBUTE_SWITCH_TO_DESKTOP,
	}
};
static TokenData bar_tok = {
	.string = "bar",
	.token = {
		.type = TOKEN_BAR,
		.value = 0,
	}
};
static TokenData background_tok = {
	.string = "background",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_BACKGROUND,
	}
};
static TokenData foreground_tok = {
	.string = "foreground",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_FOREGROUND,
	}
};
static TokenData font_tok = {
	.string = "font",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_FONT,
	}
};
static TokenData font_size_tok = {
	.string = "font_size",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_FONT_SIZE,
	}
};
static TokenData position_tok = {
	.string = "position",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_POSITION,
	}
};
static TokenData position_tok = {
	.string = "bar_position",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_POSITION,
	}
};
static TokenData bar_width_tok = {
	.string = "bar_width",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_BAR_WIDTH,
	}
};
static TokenData bar_pad_tok = {
	.string = "bar_pad",
	.token = {
		.type = TOKEN_BAR_ATTRIBUTE,
		.value = ATTRIBUTE_BAR_PAD,
	}
};
static TokenData position_left_tok = {
	.string = "left",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_LEFT,
	}
};
static TokenData position_up_tok = {
	.string = "up",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_TOP,
	}
};
static TokenData position_right_tok = {
	.string = "right",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_RIGHT,
	}
};
static TokenData position_down_tok = {
	.string = "down",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_BOTTOM,
	}
};
static TokenData position_top_tok = {
	.string = "top",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_TOP,
	}
};
static TokenData position_bottom_tok = {
	.string = "bottom",
	.token = {
		.type = TOKEN_POSITION,
		.value = BAR_BOTTOM,
	}
};
static TokenData *hashtable[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&attr_key,
	NULL,
	&send_to_desktop_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&mod_alt,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&key_f10,
	&key_f11,
	&key_f12,
	&key_f13,
	&key_f14,
	&key_f15,
	&key_f16,
	&key_f17,
	&key_f18,
	&key_f19,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&position_left_tok,
	NULL,
	NULL,
	&position_right_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	&position_up_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tk_focus_prev,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bar_width_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tk_reveal_all,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bar_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&font_size_tok,
	NULL,
	&tk_reload,
	NULL,
	NULL,
	NULL,
	NULL,
	&background_tok,
	&position_top_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&space,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&plus,
	NULL,
	&attr_modifier,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&equal,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&key_f1,
	&key_f2,
	&key_f3,
	&key_f4,
	&key_f5,
	&key_f6,
	&key_f7,
	&key_f8,
	&key_f9,
	NULL,
	NULL,
	NULL,
	&tk_quit,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&key_a,
	&key_b,
	&key_c,
	&key_d,
	&key_e,
	&key_f,
	&key_g,
	&key_h,
	&key_i,
	&key_j,
	&key_k,
	&key_l,
	&key_m,
	&key_n,
	&key_o,
	&key_p,
	&key_q,
	&key_r,
	&key_s,
	&key_t,
	&key_u,
	&key_v,
	&key_w,
	&key_x,
	&key_y,
	&key_z,
	&lbrace,
	NULL,
	&rbrace,
	NULL,
	NULL,
	&tk_focus_next,
	NULL,
	NULL,
	&bar_pad_tok,
	NULL,
	&font_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&attr_arg,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&switch_to_desktop_tok,
	NULL,
	&position_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&position_bottom_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&mod_ctrl,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&position_down_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	&foreground_tok,
	&mod_shift,
	NULL,
	NULL,
	NULL,
	NULL,
	&tk_spawn,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tk_kill,
	NULL,
	NULL,
	NULL,
	&key_f20,
	&key_f21,
	&key_f22,
	&key_f23,
	&key_f24,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&enter,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&mod_win,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&position_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&desktops_tok,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static u16 hash(char *s) {
	u16 h = 445;
	while (*s != '\0') {
		h = h * 395 + *s;
		s++;
	}
	return h;
}

Token getToken(char *s) {
	u16 h = hash(s);
	h = h & (hashtable_size - 1);
	if (hashtable[h] && (strcmp(s, hashtable[h]->string) == 0)) {
		return hashtable[h]->token;
	}
	return (Token) {
		.type = TOKEN_INVALID,
		.value = (uptr) s
	};
}
