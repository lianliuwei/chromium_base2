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
	'../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
      ],      
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
      'defines': [
        'BASE_EX_IMPLEMENTATION',
      ],
      'sources': [
        'base_ex_export.h',
        'thread_mfc.h',
        'thread_mfc.cc',
        'message_loop_mfc.h',
        'message_loop_mfc.cc',
        'message_pump_mfc.h',
        'message_pump_mfc.cc',
        'call_run_loop_hack.h',
        'call_run_loop_hack.cc',
      ],
    },
  ],
}