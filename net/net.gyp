# dump net project for ui.gyp need
{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
    'target_name': 'net',
    'type': 'static_library',
    'dependencies': [
      '../base/base.gyp:base',
      '../base/base.gyp:base_i18n',
      '../third_party/icu/icu.gyp:icui18n',
      '../third_party/icu/icu.gyp:icuuc',
    ],
    'sources': [
      'base/big_endian.cc',
      'base/big_endian.h',
      'base/escape.cc',
      'base/escape.h',
      'base/net_util.cc',
      'base/net_util.h',
      'base/net_export.h',
      'base/registry_controlled_domains/registry_controlled_domain.cc',
      'base/registry_controlled_domains/registry_controlled_domain.h',
    ],
    },
   ],
}