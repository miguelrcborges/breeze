#include "c.h"

static usize hashtable_size = 256;
static TokenData lbrace = {
	.string = string("{"),
	.token = {
		.type = TOKEN_LBRACE,
		.value = 0,
	}
};
static TokenData rbrace = {
	.string = string("}"),
	.token = {
		.type = TOKEN_RBRACE,
		.value = 0,
	}
};
static TokenData plus = {
	.string = string("+"),
	.token = {
		.type = TOKEN_PLUS,
		.value = 0,
	}
};
static TokenData equal = {
	.string = string("="),
	.token = {
		.type = TOKEN_EQUAL,
		.value = 0,
	}
};
static TokenData tk_spawn = {
	.string = string("spawn"),
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_SPAWN,
	}
};
static TokenData tk_reload = {
	.string = string("reload"),
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_RELOAD,
	}
};
static TokenData tk_quit = {
	.string = string("quit"),
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_QUIT,
	}
};
static TokenData tk_kill = {
	.string = string("kill"),
	.token = {
		.type = TOKEN_ACTION,
		.value = ACTION_KILL,
	}
};
static TokenData mod_alt = {
	.string = string("alt"),
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 1,
	}
};
static TokenData mod_ctrl = {
	.string = string("ctrl"),
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 2,
	}
};
static TokenData mod_shift = {
	.string = string("shift"),
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 4,
	}
};
static TokenData mod_win = {
	.string = string("win"),
	.token = {
		.type = TOKEN_MODIFIER,
		.value = 8,
	}
};
static TokenData attr_key = {
	.string = string("key"),
	.token = {
		.type = TOKEN_ATTRIBUTE,
		.value = ATTRIBUTE_KEY,
	}
};
static TokenData attr_modifier = {
	.string = string("modifier"),
	.token = {
		.type = TOKEN_ATTRIBUTE,
		.value = ATTRIBUTE_MODIFIER,
	}
};
static TokenData attr_arg = {
	.string = string("arg"),
	.token = {
		.type = TOKEN_ATTRIBUTE,
		.value = ATTRIBUTE_ARG,
	}
};
static TokenData key_0 = {
	.string = string("0"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x30,
	}
};
static TokenData key_1 = {
	.string = string("1"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x31,
	}
};
static TokenData key_2 = {
	.string = string("2"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x32,
	}
};
static TokenData key_3 = {
	.string = string("3"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x33,
	}
};
static TokenData key_4 = {
	.string = string("4"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x34,
	}
};
static TokenData key_5 = {
	.string = string("5"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x35,
	}
};
static TokenData key_6 = {
	.string = string("6"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x36,
	}
};
static TokenData key_7 = {
	.string = string("7"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x37,
	}
};
static TokenData key_8 = {
	.string = string("8"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x38,
	}
};
static TokenData key_9 = {
	.string = string("9"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x39,
	}
};
static TokenData key_a = {
	.string = string("a"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x41,
	}
};
static TokenData key_b = {
	.string = string("b"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x42,
	}
};
static TokenData key_c = {
	.string = string("c"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x43,
	}
};
static TokenData key_d = {
	.string = string("d"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x44,
	}
};
static TokenData key_e = {
	.string = string("e"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x45,
	}
};
static TokenData key_f = {
	.string = string("f"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x46,
	}
};
static TokenData key_g = {
	.string = string("g"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x47,
	}
};
static TokenData key_h = {
	.string = string("h"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x48,
	}
};
static TokenData key_i = {
	.string = string("i"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x49,
	}
};
static TokenData key_j = {
	.string = string("j"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4a,
	}
};
static TokenData key_k = {
	.string = string("k"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4b,
	}
};
static TokenData key_l = {
	.string = string("l"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4c,
	}
};
static TokenData key_m = {
	.string = string("m"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4d,
	}
};
static TokenData key_n = {
	.string = string("n"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4e,
	}
};
static TokenData key_o = {
	.string = string("o"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x4f,
	}
};
static TokenData key_p = {
	.string = string("p"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x50,
	}
};
static TokenData key_q = {
	.string = string("q"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x51,
	}
};
static TokenData key_r = {
	.string = string("r"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x52,
	}
};
static TokenData key_s = {
	.string = string("s"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x53,
	}
};
static TokenData key_t = {
	.string = string("t"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x54,
	}
};
static TokenData key_u = {
	.string = string("u"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x55,
	}
};
static TokenData key_v = {
	.string = string("v"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x56,
	}
};
static TokenData key_w = {
	.string = string("w"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x57,
	}
};
static TokenData key_x = {
	.string = string("x"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x58,
	}
};
static TokenData key_y = {
	.string = string("y"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x59,
	}
};
static TokenData key_z = {
	.string = string("z"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x5a,
	}
};
static TokenData key_f1 = {
	.string = string("f1"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x70,
	}
};
static TokenData key_f2 = {
	.string = string("f2"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x71,
	}
};
static TokenData key_f3 = {
	.string = string("f3"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x72,
	}
};
static TokenData key_f4 = {
	.string = string("f4"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x73,
	}
};
static TokenData key_f5 = {
	.string = string("f5"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x74,
	}
};
static TokenData key_f6 = {
	.string = string("f6"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x75,
	}
};
static TokenData key_f7 = {
	.string = string("f7"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x76,
	}
};
static TokenData key_f8 = {
	.string = string("f8"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x77,
	}
};
static TokenData key_f9 = {
	.string = string("f9"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x78,
	}
};
static TokenData key_f10 = {
	.string = string("f10"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x79,
	}
};
static TokenData key_f11 = {
	.string = string("f11"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7a,
	}
};
static TokenData key_f12 = {
	.string = string("f12"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7b,
	}
};
static TokenData key_f13 = {
	.string = string("f13"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7c,
	}
};
static TokenData key_f14 = {
	.string = string("f14"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7d,
	}
};
static TokenData key_f15 = {
	.string = string("f15"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7e,
	}
};
static TokenData key_f16 = {
	.string = string("f16"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x7f,
	}
};
static TokenData key_f17 = {
	.string = string("f17"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x80,
	}
};
static TokenData key_f18 = {
	.string = string("f18"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x81,
	}
};
static TokenData key_f19 = {
	.string = string("f19"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x82,
	}
};
static TokenData key_f20 = {
	.string = string("f20"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x83,
	}
};
static TokenData key_f21 = {
	.string = string("f21"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x84,
	}
};
static TokenData key_f22 = {
	.string = string("f22"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x85,
	}
};
static TokenData key_f23 = {
	.string = string("f23"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x86,
	}
};
static TokenData key_f24 = {
	.string = string("f24"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x87,
	}
};
static TokenData key_left = {
	.string = string("left"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x25,
	}
};
static TokenData key_up = {
	.string = string("up"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x26,
	}
};
static TokenData key_right = {
	.string = string("right"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x27,
	}
};
static TokenData key_down = {
	.string = string("down"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x27,
	}
};
static TokenData enter = {
	.string = string("enter"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x0D,
	}
};
static TokenData space = {
	.string = string("space"),
	.token = {
		.type = TOKEN_KEY,
		.value = 0x20,
	}
};
static TokenData *hashtable[] = {
	&tk_kill,
	NULL,
	&key_f20,
	&key_f21,
	&key_f22,
	&key_f23,
	&key_f24,
	NULL,
	NULL,
	&plus,
	NULL,
	NULL,
	NULL,
	&mod_ctrl,
	&key_0,
	&key_1,
	&key_2,
	&key_3,
	&key_4,
	&key_5,
	&key_6,
	&key_7,
	&key_8,
	&key_9,
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
	&space,
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
	&attr_modifier,
	&key_right,
	&tk_reload,
	NULL,
	&key_up,
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
	NULL,
	NULL,
	&mod_shift,
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
	&key_f1,
	&key_f2,
	&key_f3,
	&key_f4,
	&key_f5,
	&key_f6,
	&key_f7,
	&key_f8,
	&key_f9,
	&enter,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&attr_key,
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
	&tk_quit,
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
	&key_down,
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
	&key_left,
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
	&mod_win,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static u16 hash(string s) {
	u16 h = 226;
	for (usize i = 0; i < s.len; ++i) {
		h = h * 95 + s.str[i];
	}
	return h;
}

Token getToken(string *s) {
	u16 h = hash(*s);
	h = h & (hashtable_size - 1);
	if (hashtable[h] && string_equal(*s, hashtable[h]->string)) {
		return hashtable[h]->token;
	}
	return (Token) {
		.type = TOKEN_INVALID,
		.value = (uptr) s
	};
}
