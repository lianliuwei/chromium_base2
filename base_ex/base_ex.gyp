{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'base_ex',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
      ],      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
      'sources': [
        'thread_mfc.h',
        'thread_mfc.cc',
        'message_loop_mfc.h',
        'message_loop_mfc.cc',
        'message_pump_mfc.h',
        'message_pump_mfc.cc',
      ],
    },
  ],
}