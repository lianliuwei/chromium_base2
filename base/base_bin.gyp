# only support dll bin reuse now.
{
  'variables': {
    'chromium_code': 1,
    'bin_path': '..\\build\\$(ConfigurationName)',
    'lib_path': '<(bin_path)\\lib',
    'dest_lib_path': '$(OutDir)\\lib\\',
  },
  
  'includes': [
    'base.gypi',
  ],
  
  'targets': [
    {
      'target_name': 'base_bin',
      'type': 'none',
      'variables': {
        'base_target': 1,
      },
      
      'dependencies': [
      ],
 
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'base.lib',
            ],
            'AdditionalLibraryDirectories': [
              '<(dest_lib_path)'
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)\\base.dll',
            '<(bin_path)\\base.dll.pdb',
          ],
        },
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\base.lib'
          ],
        },
      ],
      'sources': [
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
    {
      'target_name': 'base_i18n_bin',
      'type': 'none',    
      'dependencies': [
        'base_bin',
      ],
      'export_dependent_settings': [
        'base_bin',
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'base_i18n.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)\\base_i18n.dll',
            '<(bin_path)\\base_i18n.dll.pdb',
          ],
        },
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\base_i18n.lib'
          ],
        },
      ],
      'soucres': [
        'i18n/base_i18n_export.h',
        'i18n/bidi_line_iterator.cc',
        'i18n/bidi_line_iterator.h',
        'i18n/break_iterator.cc',
        'i18n/break_iterator.h',
        'i18n/char_iterator.cc',
        'i18n/char_iterator.h',
        'i18n/case_conversion.cc',
        'i18n/case_conversion.h',
        'i18n/file_util_icu.cc',
        'i18n/file_util_icu.h',
        'i18n/icu_encoding_detection.cc',
        'i18n/icu_encoding_detection.h',
        'i18n/icu_string_conversions.cc',
        'i18n/icu_string_conversions.h',
        'i18n/icu_util.cc',
        'i18n/icu_util.h',
        'i18n/number_formatting.cc',
        'i18n/number_formatting.h',
        'i18n/rtl.cc',
        'i18n/rtl.h',
        'i18n/string_search.cc',
        'i18n/string_search.h',
        'i18n/time_formatting.cc',
        'i18n/time_formatting.h',
      ],
    },
    {
      'target_name': 'base_prefs_bin',
      'type': 'none',    
      'dependencies': [
        'base_bin',
      ],
      'export_dependent_settings': [
        'base_bin',
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'base_prefs.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(bin_path)\\base_prefs.dll',
            '<(bin_path)\\base_prefs.dll.pdb',
          ],
        },
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\base_prefs.lib'
          ],
        },
      ],
      'sources': [
        'prefs/base_prefs_export.h',
        'prefs/default_pref_store.cc',
        'prefs/default_pref_store.h',
        'prefs/json_pref_store.cc',
        'prefs/json_pref_store.h',
        'prefs/overlay_user_pref_store.cc',
        'prefs/overlay_user_pref_store.h',
        'prefs/persistent_pref_store.h',
        'prefs/pref_change_registrar.cc',
        'prefs/pref_change_registrar.h',
        'prefs/pref_member.cc',
        'prefs/pref_member.h',
        'prefs/pref_notifier.h',
        'prefs/pref_notifier_impl.cc',
        'prefs/pref_notifier_impl.h',
        'prefs/pref_observer.h',
        'prefs/pref_registry.cc',
        'prefs/pref_registry.h',
        'prefs/pref_registry_simple.cc',
        'prefs/pref_registry_simple.h',
        'prefs/pref_service.cc',
        'prefs/pref_service.h',
        'prefs/pref_service_builder.cc',
        'prefs/pref_service_builder.h',
        'prefs/pref_store.cc',
        'prefs/pref_store.h',
        'prefs/pref_value_map.cc',
        'prefs/pref_value_map.h',
        'prefs/pref_value_store.cc',
        'prefs/pref_value_store.h',
        'prefs/value_map_pref_store.cc',
        'prefs/value_map_pref_store.h',
      ],
    },
    {
      'target_name': 'base_prefs_test_support_bin',
      'type': 'none',    
      'dependencies': [
        'base_bin',
        'base_prefs_bin',
        '../testing/gmock.gyp:gmock',
      ],
      'export_dependent_settings': [
        'base_bin',
        'base_prefs_bin'
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'base_prefs_test_support.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\base_prefs_test_support.lib'
          ],
        },
      ],
      'sources': [
        'prefs/mock_pref_change_callback.cc',
        'prefs/pref_store_observer_mock.cc',
        'prefs/pref_store_observer_mock.h',
        'prefs/testing_pref_service.cc',
        'prefs/testing_pref_service.h',
        'prefs/testing_pref_store.cc',
        'prefs/testing_pref_store.h',
      ],
    },
    {
      'target_name': 'test_support_base_bin',
      'type': 'none',    
      'dependencies': [
        'base_bin',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
      ],
      'export_dependent_settings': [
        'base_bin',
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'test_support_base.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\test_support_base.lib'
          ],
        },
      ],
      'sources': [
        'perftimer.cc',
        'test/expectations/expectation.cc',
        'test/expectations/expectation.h',
        'test/expectations/parser.cc',
        'test/expectations/parser.h',
        'test/mock_chrome_application_mac.h',
        'test/mock_chrome_application_mac.mm',
        'test/mock_devices_changed_observer.cc',
        'test/mock_devices_changed_observer.h',
        'test/mock_time_provider.cc',
        'test/mock_time_provider.h',
        'test/multiprocess_test.cc',
        'test/multiprocess_test.h',
        'test/multiprocess_test_android.cc',
        'test/null_task_runner.cc',
        'test/null_task_runner.h',
        'test/perf_test_suite.cc',
        'test/perf_test_suite.h',
        'test/scoped_locale.cc',
        'test/scoped_locale.h',
        'test/scoped_path_override.cc',
        'test/scoped_path_override.h',
        'test/sequenced_task_runner_test_template.cc',
        'test/sequenced_task_runner_test_template.h',
        'test/sequenced_worker_pool_owner.cc',
        'test/sequenced_worker_pool_owner.h',
        'test/simple_test_clock.cc',
        'test/simple_test_clock.h',
        'test/simple_test_tick_clock.cc',
        'test/simple_test_tick_clock.h',
        'test/task_runner_test_template.cc',
        'test/task_runner_test_template.h',
        'test/test_file_util.h',
        'test/test_file_util_linux.cc',
        'test/test_file_util_mac.cc',
        'test/test_file_util_posix.cc',
        'test/test_file_util_win.cc',
        'test/test_listener_ios.h',
        'test/test_listener_ios.mm',
        'test/test_pending_task.cc',
        'test/test_pending_task.h',
        'test/test_process_killer_win.cc',
        'test/test_process_killer_win.h',
        'test/test_reg_util_win.cc',
        'test/test_reg_util_win.h',
        'test/test_shortcut_win.cc',
        'test/test_shortcut_win.h',
        'test/test_simple_task_runner.cc',
        'test/test_simple_task_runner.h',
        'test/test_suite.cc',
        'test/test_suite.h',
        'test/test_support_android.cc',
        'test/test_support_android.h',
        'test/test_support_ios.h',
        'test/test_support_ios.mm',
        'test/test_switches.cc',
        'test/test_switches.h',
        'test/test_timeouts.cc',
        'test/test_timeouts.h',
        'test/thread_test_helper.cc',
        'test/thread_test_helper.h',
        'test/trace_event_analyzer.cc',
        'test/trace_event_analyzer.h',
        'test/values_test_util.cc',
        'test/values_test_util.h',
      ],
    },
    {
      'target_name': 'test_support_perf_bin',
      'type': 'none',    
      'dependencies': [
        'base_bin',
        'base_prefs_bin',
        '../testing/gtest.gyp:gtest',
      ],
      'export_dependent_settings': [
        'base_bin',
        'base_prefs_bin',
      ],      
      'direct_dependent_settings': {
        'defines': [
          'PERF_TEST',
        ],
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'test_support_perf.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\test_support_perf.lib'
          ],
        },
      ],
      'sources': [
        'perftimer.cc',
        'test/run_all_perftests.cc',
      ],
    },
    {
      'target_name': 'base_static_bin',
      'type': 'none',
      'dependencies': [
      ],
      'export_dependent_settings': [
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'base_static.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\base_static.lib',
          ],
        },
      ],
      'sources': [
        'base_switches.cc',
        'base_switches.h',
        'win/pe_image.cc',
        'win/pe_image.h',
      ],
    },
    {
      'target_name': 'run_all_unittests_bin',
      'type': 'none',    
      'dependencies': [
        'test_support_base_bin',
      ],      
      'direct_dependent_settings': {
        'msvs_settings':{
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'run_all_unittests.lib',
            ],
          },
        },
      },
      'copies': [
        {
          'destination': '<(dest_lib_path)',
          'files': [
            '<(lib_path)\\run_all_unittests.lib'
          ],
        },
      ],
      'sources': [
        'test/run_all_unittests.cc',
      ],
    },
  ],
}