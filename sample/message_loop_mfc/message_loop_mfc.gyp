{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'message_loop_mfc',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base_ex/base_ex.gyp:base_ex',
      ],
      'include_dirs': [
        '.',
        'app/',
      ], 
      'sources': [
        'app/app.h',
        'app/app.cc',
        'app/stdafx.h',
        'app/stdafx.cc',
        'app/target_version.h',
        'ui/main_frame.h',
        'ui/main_frame.cc',
        'ui/test_view.h',
        'ui/test_view.cc',
        'ui/form_view_ex.h',
        'resources/resource.h',
        'resources/app.ico',
        'resources/main_frame.rc',
        'resources/mfc_res.rc',
        'resources/version.rc',
      ],
      
      'msvs_precompiled_header': 'app/stdafx.h',
      'msvs_precompiled_source': 'app/stdafx.cc',
      
      'msvs_configuration_attributes': {
        'conditions': [
          ['component=="shared_library"', {
            'UseOfMFC': '2',  # Shared DLL
          },{
            'UseOfMFC': '1',  # Static
          }],
        ],
      },
      'msvs_settings': {
        'VCLinkerTool': {
          #   2 == /SUBSYSTEM:WINDOWS
          'SubSystem': '2',
        },
      },
    },
  ],
}