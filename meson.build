project(
  'tbo',
  'c',
  version: '2.0',
  default_options: ['c_std=gnu11']
)

if get_option('nls')
  subdir('po')
endif

cc = meson.get_compiler('c')

gtk_dep = dependency('gtk4', version: '>= 4.8')
cairo_dep = dependency('cairo')
rsvg_dep = dependency('librsvg-2.0')
math_dep = cc.find_library('m', required: false)

deps = [gtk_dep, cairo_dep, rsvg_dep, math_dep]

data_dir = join_paths(get_option('prefix'), get_option('datadir'), 'tbo')
locale_dir = join_paths(get_option('prefix'), get_option('localedir'))

conf = configuration_data()
conf.set_quoted('GETTEXT_PACKAGE', 'tbo')
conf.set_quoted('VERSION', meson.project_version())
conf.set10('ENABLE_NLS', get_option('nls'))
configure_file(output: 'config.h', configuration: conf)
configured_icon_png = configure_file(input: 'data/icon.png', output: 'tbo.png', copy: true)

add_project_arguments(
  '-DG_LOG_DOMAIN="tbo"',
  '-DGNOMELOCALEDIR="' + locale_dir + '"',
  '-DDATA_DIR="' + data_dir + '"',
  '-DSOURCE_DATA_DIR="' + join_paths(meson.project_source_root(), 'data') + '"',
  language: 'c'
)

inc = include_directories('src')

common_sources = files(
  'src/tbo-window.c',
  'src/tbo-file-dialog.c',
  'src/tbo-widget.c',
  'src/comic.c',
  'src/comic-new-dialog.c',
  'src/comic-saveas-dialog.c',
  'src/comic-open-dialog.c',
  'src/frame.c',
  'src/page.c',
  'src/ui-menu.c',
  'src/doodle-treeview.c',
  'src/tbo-tooltip.c',
  'src/export.c',
  'src/dnd.c',
  'src/comic-load.c',
  'src/tbo-utils.c',
  'src/tbo-files.c',
  'src/tbo-ui-utils.c',
  'src/tbo-object-base.c',
  'src/tbo-object-svg.c',
  'src/tbo-object-text.c',
  'src/tbo-object-pixmap.c',
  'src/tbo-object-group.c',
  'src/tbo-tool-base.c',
  'src/tbo-tool-selector.c',
  'src/tbo-tool-frame.c',
  'src/tbo-tool-doodle.c',
  'src/tbo-tool-bubble.c',
  'src/tbo-tool-text.c',
  'src/tbo-toolbar.c',
  'src/tbo-drawing.c',
  'src/tbo-undo.c'
)

executable(
  'tbo',
  common_sources + files('src/tbo.c'),
  dependencies: deps,
  include_directories: inc,
  install: true
)

if get_option('tests')
load_render_check = executable(
  'load-render-check',
  common_sources + files('tests/load_render_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_tool_check = executable(
  'frame-tool-check',
  common_sources + files('tests/frame_tool_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

save_roundtrip_check = executable(
  'save-roundtrip-check',
  common_sources + files('tests/save_roundtrip_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

asset_bounds_check = executable(
  'asset-bounds-check',
  common_sources + files('tests/asset_bounds_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

document_state_reset_check = executable(
  'document-state-reset-check',
  common_sources + files('tests/document_state_reset_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

model_current_state_check = executable(
  'model-current-state-check',
  common_sources + files('tests/model_current_state_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_gobject_lifetime_check = executable(
  'frame-gobject-lifetime-check',
  common_sources + files('tests/frame_gobject_lifetime_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

page_gobject_lifetime_check = executable(
  'page-gobject-lifetime-check',
  common_sources + files('tests/page_gobject_lifetime_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

comic_gobject_lifetime_check = executable(
  'comic-gobject-lifetime-check',
  common_sources + files('tests/comic_gobject_lifetime_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

ascii_locale_roundtrip_check = executable(
  'ascii-locale-roundtrip-check',
  common_sources + files('tests/ascii_locale_roundtrip_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

legacy_decimal_load_check = executable(
  'legacy-decimal-load-check',
  common_sources + files('tests/legacy_decimal_load_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

dirty_state_check = executable(
  'dirty-state-check',
  common_sources + files('tests/dirty_state_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

resize_rotate_undo_check = executable(
  'resize-rotate-undo-check',
  common_sources + files('tests/resize_rotate_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

xml_roundtrip_check = executable(
  'xml-roundtrip-check',
  common_sources + files('tests/xml_roundtrip_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

invalid_tbo_check = executable(
  'invalid-tbo-check',
  common_sources + files('tests/invalid_tbo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

i18n_check = executable(
  'i18n-check',
  common_sources + files('tests/i18n_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_result_check = executable(
  'export-result-check',
  common_sources + files('tests/export_result_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

page_status_check = executable(
  'page-status-check',
  common_sources + files('tests/page_status_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

clone_dirty_check = executable(
  'clone-dirty-check',
  common_sources + files('tests/clone_dirty_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

group_clone_check = executable(
  'group-clone-check',
  common_sources + files('tests/group_clone_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

group_selection_cleanup_check = executable(
  'group-selection-cleanup-check',
  common_sources + files('tests/group_selection_cleanup_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

zoom_bounds_check = executable(
  'zoom-bounds-check',
  common_sources + files('tests/zoom_bounds_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_default_color_check = executable(
  'frame-default-color-check',
  common_sources + files('tests/frame_default_color_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_scale_check = executable(
  'export-scale-check',
  common_sources + files('tests/export_scale_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_view_coordinate_mapping_check = executable(
  'frame-view-coordinate-mapping-check',
  common_sources + files('tests/frame_view_coordinate_mapping_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

key_binder_scope_check = executable(
  'key-binder-scope-check',
  common_sources + files('tests/key_binder_scope_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_extension_hint_check = executable(
  'export-extension-hint-check',
  common_sources + files('tests/export_extension_hint_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

doodle_dir_error_check = executable(
  'doodle-dir-error-check',
  common_sources + files('tests/doodle_dir_error_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

comic_template_check = executable(
  'comic-template-check',
  common_sources + files('tests/comic_template_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

a4_pdf_export_check = executable(
  'a4-pdf-export-check',
  common_sources + files('tests/a4_pdf_export_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

autosave_recovery_check = executable(
  'autosave-recovery-check',
  common_sources + files('tests/autosave_recovery_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

recent_projects_check = executable(
  'recent-projects-check',
  common_sources + files('tests/recent_projects_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

dirty_title_check = executable(
  'dirty-title-check',
  common_sources + files('tests/dirty_title_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

mode_status_check = executable(
  'mode-status-check',
  common_sources + files('tests/mode_status_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_scope_check = executable(
  'export-scope-check',
  common_sources + files('tests/export_scope_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

dnd_feedback_check = executable(
  'dnd-feedback-check',
  common_sources + files('tests/dnd_feedback_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

shortcuts_guide_check = executable(
  'shortcuts-guide-check',
  common_sources + files('tests/shortcuts_guide_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

page_duplicate_check = executable(
  'page-duplicate-check',
  common_sources + files('tests/page_duplicate_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

page_reorder_check = executable(
  'page-reorder-check',
  common_sources + files('tests/page_reorder_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

assets_browser_search_check = executable(
  'assets-browser-search-check',
  common_sources + files('tests/assets_browser_search_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

doodle_raster_catalog_check = executable(
  'doodle-raster-catalog-check',
  common_sources + files('tests/doodle_raster_catalog_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_page_range_check = executable(
  'export-page-range-check',
  common_sources + files('tests/export_page_range_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_guided_dialog_check = executable(
  'export-guided-dialog-check',
  common_sources + files('tests/export_guided_dialog_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

keyboard_accessibility_check = executable(
  'keyboard-accessibility-check',
  common_sources + files('tests/keyboard_accessibility_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

theme_respect_check = executable(
  'theme-respect-check',
  common_sources + files('tests/theme_respect_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

enter_frame_key_check = executable(
  'enter-frame-key-check',
  common_sources + files('tests/enter_frame_key_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_count_status_check = executable(
  'frame-count-status-check',
  common_sources + files('tests/frame_count_status_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_formats_check = executable(
  'export-formats-check',
  common_sources + files('tests/export_formats_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

raster_render_check = executable(
  'raster-render-check',
  common_sources + files('tests/raster_render_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

invalid_tbo_variants_check = executable(
  'invalid-tbo-variants-check',
  common_sources + files('tests/invalid_tbo_variants_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

tooltip_scope_check = executable(
  'tooltip-scope-check',
  common_sources + files('tests/tooltip_scope_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

toolbar_save_button_check = executable(
  'toolbar-save-button-check',
  common_sources + files('tests/toolbar_save_button_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

status_hierarchy_check = executable(
  'status-hierarchy-check',
  common_sources + files('tests/status_hierarchy_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

create_undo_check = executable(
  'create-undo-check',
  common_sources + files('tests/create_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

page_undo_check = executable(
  'page-undo-check',
  common_sources + files('tests/page_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

text_undo_check = executable(
  'text-undo-check',
  common_sources + files('tests/text_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

saveas_filename_check = executable(
  'saveas-filename-check',
  common_sources + files('tests/saveas_filename_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

object_delete_undo_check = executable(
  'object-delete-undo-check',
  common_sources + files('tests/object_delete_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

frame_delete_undo_check = executable(
  'frame-delete-undo-check',
  common_sources + files('tests/frame_delete_undo_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

undo_branch_reset_check = executable(
  'undo-branch-reset-check',
  common_sources + files('tests/undo_branch_reset_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

document_replace_failure_check = executable(
  'document-replace-failure-check',
  common_sources + files('tests/document_replace_failure_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

tutorial_open_check = executable(
  'tutorial-open-check',
  common_sources + files('tests/tutorial_open_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

export_empty_comic_check = executable(
  'export-empty-comic-check',
  common_sources + files('tests/export_empty_comic_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

save_failure_check = executable(
  'save-failure-check',
  common_sources + files('tests/save_failure_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

undo_saved_state_check = executable(
  'undo-saved-state-check',
  common_sources + files('tests/undo_saved_state_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

recent_missing_file_check = executable(
  'recent-missing-file-check',
  common_sources + files('tests/recent_missing_file_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

recover_file_failure_check = executable(
  'recover-file-failure-check',
  common_sources + files('tests/recover_file_failure_check.c'),
  dependencies: deps,
  include_directories: inc,
  install: false
)

test(
  'load-render-check',
  load_render_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-tool-check',
  frame_tool_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'save-roundtrip-check',
  save_roundtrip_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'asset-bounds-check',
  asset_bounds_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'document-state-reset-check',
  document_state_reset_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'model-current-state-check',
  model_current_state_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-gobject-lifetime-check',
  frame_gobject_lifetime_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'page-gobject-lifetime-check',
  page_gobject_lifetime_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'comic-gobject-lifetime-check',
  comic_gobject_lifetime_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'ascii-locale-roundtrip-check',
  ascii_locale_roundtrip_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'legacy-decimal-load-check',
  legacy_decimal_load_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'dirty-state-check',
  dirty_state_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'resize-rotate-undo-check',
  resize_rotate_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'xml-roundtrip-check',
  xml_roundtrip_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'invalid-tbo-check',
  invalid_tbo_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'i18n-check',
  i18n_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-result-check',
  export_result_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'page-status-check',
  page_status_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'clone-dirty-check',
  clone_dirty_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'group-clone-check',
  group_clone_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'group-selection-cleanup-check',
  group_selection_cleanup_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'zoom-bounds-check',
  zoom_bounds_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-default-color-check',
  frame_default_color_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-scale-check',
  export_scale_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-view-coordinate-mapping-check',
  frame_view_coordinate_mapping_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'key-binder-scope-check',
  key_binder_scope_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-extension-hint-check',
  export_extension_hint_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'doodle-dir-error-check',
  doodle_dir_error_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'comic-template-check',
  comic_template_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'a4-pdf-export-check',
  a4_pdf_export_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'autosave-recovery-check',
  autosave_recovery_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'recent-projects-check',
  recent_projects_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'dirty-title-check',
  dirty_title_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'mode-status-check',
  mode_status_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-scope-check',
  export_scope_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'dnd-feedback-check',
  dnd_feedback_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'shortcuts-guide-check',
  shortcuts_guide_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'page-duplicate-check',
  page_duplicate_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'page-reorder-check',
  page_reorder_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'assets-browser-search-check',
  assets_browser_search_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'doodle-raster-catalog-check',
  doodle_raster_catalog_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-page-range-check',
  export_page_range_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-guided-dialog-check',
  export_guided_dialog_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'keyboard-accessibility-check',
  keyboard_accessibility_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'theme-respect-check',
  theme_respect_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'enter-frame-key-check',
  enter_frame_key_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-count-status-check',
  frame_count_status_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-formats-check',
  export_formats_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'raster-render-check',
  raster_render_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'invalid-tbo-variants-check',
  invalid_tbo_variants_check,
  args: [join_paths(meson.project_source_root(), 'data', 'tut.tbo')],
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'tooltip-scope-check',
  tooltip_scope_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'toolbar-save-button-check',
  toolbar_save_button_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'status-hierarchy-check',
  status_hierarchy_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'create-undo-check',
  create_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'page-undo-check',
  page_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'text-undo-check',
  text_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'saveas-filename-check',
  saveas_filename_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'object-delete-undo-check',
  object_delete_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'frame-delete-undo-check',
  frame_delete_undo_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'undo-branch-reset-check',
  undo_branch_reset_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'document-replace-failure-check',
  document_replace_failure_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'tutorial-open-check',
  tutorial_open_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'export-empty-comic-check',
  export_empty_comic_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'save-failure-check',
  save_failure_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'undo-saved-state-check',
  undo_saved_state_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'recent-missing-file-check',
  recent_missing_file_check,
  env: ['G_DEBUG=fatal-criticals']
)

test(
  'recover-file-failure-check',
  recover_file_failure_check,
  env: ['G_DEBUG=fatal-criticals']
)
endif

installable_data_files = ['data/tut.tbo', 'data/icon.png']

if get_option('tutorial_pdf')
  installable_data_files += 'data/tutorial.pdf'
endif

install_data(
  installable_data_files,
  install_dir: join_paths(get_option('datadir'), 'tbo')
)

install_subdir('data/ui', install_dir: join_paths(get_option('datadir'), 'tbo'))
install_subdir('data/icons', install_dir: join_paths(get_option('datadir'), 'tbo'))
install_subdir('data/doodle', install_dir: join_paths(get_option('datadir'), 'tbo'))

install_data('data/applications/net.danigm.tbo.desktop', install_dir: join_paths(get_option('datadir'), 'applications'))

if get_option('appstream')
  install_data('data/metainfo/net.danigm.tbo.metainfo.xml', install_dir: join_paths(get_option('datadir'), 'metainfo'))
endif

if get_option('legacy_pixmap_icon')
  install_data('data/tbo.svg', install_dir: join_paths(get_option('datadir'), 'pixmaps'))
endif

install_data('data/tbo.svg', install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', 'scalable', 'apps'))
install_data(configured_icon_png, install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', '128x128', 'apps'))
