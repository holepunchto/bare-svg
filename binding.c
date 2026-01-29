#include <assert.h>
#include <bare.h>
#include <js.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "resvg.h"

#define DEFAULT_WIDTH 512
#define DEFAULT_HEIGHT 512
#define DEFAULT_DPI 96.0f

static resvg_options *g_options_with_fonts = NULL;
static resvg_options *g_options_without_fonts = NULL;

static void
bare_svg__on_finalize(js_env_t *env, void *data, void *finalize_hint) {
  free(data);
}

static resvg_options *
bare_svg__get_options(float dpi, bool load_fonts) {
  if (load_fonts) {
    if (g_options_with_fonts == NULL) {
      g_options_with_fonts = resvg_options_create();
      resvg_options_load_system_fonts(g_options_with_fonts);
    }
    resvg_options_set_dpi(g_options_with_fonts, dpi);
    return g_options_with_fonts;
  } else {
    if (g_options_without_fonts == NULL) {
      g_options_without_fonts = resvg_options_create();
    }
    resvg_options_set_dpi(g_options_without_fonts, dpi);
    return g_options_without_fonts;
  }
}

/**
 * Decode SVG string/buffer to RGBA format
 *
 * Arguments:
 * - svg: SVG data (string or buffer)
 * - options: { width?, height?, dpi?, loadFonts? }
 *
 * Returns: {width, height, data}
 */
static js_value_t *
bare_svg_decode(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  if (argc < 1) {
    err = js_throw_type_error(env, NULL, "SVG decode requires at least one argument");
    assert(err == 0);
    return NULL;
  }

  char *svg_data = NULL;
  size_t svg_len = 0;

  js_value_type_t type;
  err = js_typeof(env, argv[0], &type);
  assert(err == 0);

  if (type == js_string) {
    err = js_get_value_string_utf8(env, argv[0], NULL, 0, &svg_len);
    assert(err == 0);

    svg_data = malloc(svg_len + 1);
    err = js_get_value_string_utf8(env, argv[0], (utf8_t *) svg_data, svg_len + 1, &svg_len);
    assert(err == 0);
  } else {
    void *buffer_data;
    err = js_get_typedarray_info(env, argv[0], NULL, &buffer_data, &svg_len, NULL, NULL);
    if (err != 0) {
      err = js_throw_type_error(env, NULL, "SVG input must be string or buffer");
      assert(err == 0);
      return NULL;
    }

    svg_data = malloc(svg_len + 1);
    memcpy(svg_data, buffer_data, svg_len);
    svg_data[svg_len] = '\0';
  }

  float target_width = 0.0f;
  float target_height = 0.0f;
  float dpi = DEFAULT_DPI;
  bool load_fonts = true;

  if (argc > 1 && argv[1] != NULL) {
    js_value_type_t options_type;
    err = js_typeof(env, argv[1], &options_type);
    assert(err == 0);

    if (options_type != js_object) {
      free(svg_data);
      err = js_throw_type_error(env, NULL, "Options must be an object");
      assert(err == 0);
      return NULL;
    }

    js_value_t *width_val, *height_val, *dpi_val, *load_fonts_val;
    bool has_prop;

    err = js_has_named_property(env, argv[1], "width", &has_prop);
    assert(err == 0);
    if (has_prop) {
      err = js_get_named_property(env, argv[1], "width", &width_val);
      assert(err == 0);
      double width_dbl;
      err = js_get_value_double(env, width_val, &width_dbl);
      assert(err == 0);
      target_width = (float) width_dbl;
    }

    err = js_has_named_property(env, argv[1], "height", &has_prop);
    assert(err == 0);
    if (has_prop) {
      err = js_get_named_property(env, argv[1], "height", &height_val);
      assert(err == 0);
      double height_dbl;
      err = js_get_value_double(env, height_val, &height_dbl);
      assert(err == 0);
      target_height = (float) height_dbl;
    }

    err = js_has_named_property(env, argv[1], "dpi", &has_prop);
    assert(err == 0);
    if (has_prop) {
      err = js_get_named_property(env, argv[1], "dpi", &dpi_val);
      assert(err == 0);
      double dpi_dbl;
      err = js_get_value_double(env, dpi_val, &dpi_dbl);
      assert(err == 0);
      dpi = (float) dpi_dbl;
    }

    err = js_has_named_property(env, argv[1], "loadFonts", &has_prop);
    assert(err == 0);
    if (has_prop) {
      err = js_get_named_property(env, argv[1], "loadFonts", &load_fonts_val);
      assert(err == 0);
      err = js_get_value_bool(env, load_fonts_val, &load_fonts);
      assert(err == 0);
    }
  }

  resvg_options *opt = bare_svg__get_options(dpi, load_fonts);

  resvg_render_tree *tree = NULL;
  int parse_result = resvg_parse_tree_from_data(svg_data, svg_len, opt, &tree);
  free(svg_data);

  if (parse_result != RESVG_OK || tree == NULL) {
    const char *error_msg;
    switch (parse_result) {
    case RESVG_ERROR_NOT_AN_UTF8_STR:
      error_msg = "SVG data is not valid UTF-8";
      break;
    case RESVG_ERROR_MALFORMED_GZIP:
      error_msg = "SVG gzip data is malformed";
      break;
    case RESVG_ERROR_ELEMENTS_LIMIT_REACHED:
      error_msg = "SVG elements limit reached";
      break;
    case RESVG_ERROR_INVALID_SIZE:
      error_msg = "SVG has invalid size";
      break;
    case RESVG_ERROR_PARSING_FAILED:
      error_msg = "Failed to parse SVG";
      break;
    default:
      error_msg = "Unknown SVG parsing error";
      break;
    }
    err = js_throw_error(env, NULL, error_msg);
    assert(err == 0);
    return NULL;
  }

  resvg_size size = resvg_get_image_size(tree);
  float svg_width = size.width;
  float svg_height = size.height;

  if (svg_width <= 0.0f) svg_width = DEFAULT_WIDTH;
  if (svg_height <= 0.0f) svg_height = DEFAULT_HEIGHT;

  float aspect = svg_width / svg_height;
  float width, height;

  if (target_width > 0.0f && target_height > 0.0f) {
    width = target_width;
    height = target_height;
  } else if (target_width > 0.0f) {
    width = target_width;
    height = width / aspect;
  } else if (target_height > 0.0f) {
    height = target_height;
    width = height * aspect;
  } else {
    width = svg_width;
    height = svg_height;
  }

  int w = (int) roundf(width);
  int h = (int) roundf(height);

  size_t buffer_size = (size_t) w * h * 4;
  uint8_t *rgba = malloc(buffer_size);
  if (!rgba) {
    resvg_tree_destroy(tree);
    err = js_throw_error(env, NULL, "Memory allocation failed");
    assert(err == 0);
    return NULL;
  }

  memset(rgba, 0, buffer_size);

  float scale_x = width / svg_width;
  float scale_y = height / svg_height;
  float scale = fminf(scale_x, scale_y);

  float offset_x = (width - svg_width * scale) / 2.0f;
  float offset_y = (height - svg_height * scale) / 2.0f;

  resvg_transform transform = {
    .a = scale,
    .b = 0.0f,
    .c = 0.0f,
    .d = scale,
    .e = offset_x,
    .f = offset_y
  };

  resvg_render(tree, transform, (uint32_t) w, (uint32_t) h, (char *) rgba);

  resvg_tree_destroy(tree);

  js_value_t *result;
  err = js_create_object(env, &result);
  assert(err == 0);

  js_value_t *width_val;
  err = js_create_int64(env, w, &width_val);
  assert(err == 0);
  err = js_set_named_property(env, result, "width", width_val);
  assert(err == 0);

  js_value_t *height_val;
  err = js_create_int64(env, h, &height_val);
  assert(err == 0);
  err = js_set_named_property(env, result, "height", height_val);
  assert(err == 0);

  js_value_t *buffer;
  err = js_create_external_arraybuffer(env, rgba, buffer_size, bare_svg__on_finalize, NULL, &buffer);
  assert(err == 0);
  err = js_set_named_property(env, result, "data", buffer);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_svg_exports(js_env_t *env, js_value_t *exports) {
  int err;

#define V(name, fn) \
  { \
    js_value_t *val; \
    err = js_create_function(env, name, -1, fn, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

  V("decode", bare_svg_decode)

#undef V

  return exports;
}

BARE_MODULE(bare_svg, bare_svg_exports)
