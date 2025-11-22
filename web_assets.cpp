#include "web_assets.h"

#include "app_state.h"

#ifndef ASSET_PATH_PREFIX
#define ASSET_PATH_PREFIX "/home/mike/source/terrarium_fw/assets/"
#endif

#define ASSET_PATH(name) ASSET_PATH_PREFIX name

#define DECLARE_BINARY(name)                                 \
  extern const unsigned char name[];                         \
  extern const unsigned char name##_end[]

#define DEFINE_BINARY(name, path)                                                  \
  __asm__(".section .rodata\n"                                                  \
          ".global " #name "\n"                                                   \
          #name ":\n"                                                             \
          ".incbin \"" path "\"\n"                                              \
          ".global " #name "_end\n"                                                \
          #name "_end:\n"                                                          \
          ".byte 0\n"                                                               \
          ".section .text\n");

DECLARE_BINARY(index_html);
DECLARE_BINARY(portal_html);
DECLARE_BINARY(main_css);
DECLARE_BINARY(app_js);
DECLARE_BINARY(portal_css);
DECLARE_BINARY(portal_js);

DEFINE_BINARY(index_html, ASSET_PATH("index.html"));
DEFINE_BINARY(portal_html, ASSET_PATH("portal.html"));
DEFINE_BINARY(main_css, ASSET_PATH("main.css"));
DEFINE_BINARY(app_js, ASSET_PATH("app.js"));
DEFINE_BINARY(portal_css, ASSET_PATH("portal.css"));
DEFINE_BINARY(portal_js, ASSET_PATH("portal.js"));

namespace {

String loadTemplate(const unsigned char *start, const unsigned char *end) {
  return String(reinterpret_cast<const char *>(start));
}

void sendAsset(const unsigned char *start, const unsigned char *end, const char *contentType) {
  const size_t length = static_cast<size_t>(end - start);
  server.send_P(200, contentType, reinterpret_cast<const char *>(start), length);
}

}  // namespace

String loadIndexTemplate() {
  return loadTemplate(index_html, index_html_end);
}

String loadPortalTemplate() {
  return loadTemplate(portal_html, portal_html_end);
}

void sendMainCss() {
  sendAsset(main_css, main_css_end, "text/css");
}

void sendAppJs() {
  sendAsset(app_js, app_js_end, "application/javascript");
}

void sendPortalCss() {
  sendAsset(portal_css, portal_css_end, "text/css");
}

void sendPortalJs() {
  sendAsset(portal_js, portal_js_end, "application/javascript");
}
