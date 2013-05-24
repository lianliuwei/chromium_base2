# only support dll bin reuse now.
{
  'variables': {
    'bin_path': '..\\build\\<(build_dir_prefix)$(ConfigurationName)',
    'lib_path': '<(bin_path)\\lib',
  },
  
  'includes': [
    'base.gypi',
  ],
  'dependencies': [
    ''
  ],
  
  'targets': [
    {
      'target_name': 'base',
      'type': 'none',
      'variables': {
        'base_target': 1,
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)\\base.dll',
            '<(bin_path)\\base.dll.pdb',
          ],
        },
        {
          'destination': '$(OutDir)\\lib\\',
          'files': [
            '<(lib_path)\\base.lib'
          ],
        },
      'soucres': [
        'third_party/nspr/prcpucfg.h',
        'third_party/nspr/prcpucfg_win.h',
        'third_party/nspr/prtypes.h',
        'third_party/xdg_user_dirs/xdg_user_dir_lookup.cc',
        'third_party/xdg_user_dirs/xdg_user_dir_lookup.h',
        'auto_reset.h',
        'event_recorder.h',
        'event_recorder_stubs.cc',
        'event_recorder_win.cc',
        'linux_util.cc',
        'linux_util.h',
        'md5.cc',
        'md5.h',
        'message_pump_android.cc',
        'message_pump_android.h',
        'message_pump_glib.cc',
        'message_pump_glib.h',
        'message_pump_gtk.cc',
        'message_pump_gtk.h',
        'message_pump_io_ios.cc',
        'message_pump_io_ios.h',
        'message_pump_observer.h',
        'message_pump_aurax11.cc',
        'message_pump_aurax11.h',
        'message_pump_libevent.cc',
        'message_pump_libevent.h',
        'message_pump_mac.h',
        'message_pump_mac.mm',
        'metrics/field_trial.cc',
        'metrics/field_trial.h',
        'posix/file_descriptor_shuffle.cc',
        'posix/file_descriptor_shuffle.h',
        'sync_socket.h',
        'sync_socket_win.cc',
        'sync_socket_posix.cc',
      ],
    },
#    {
#    },
  ],
}