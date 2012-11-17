<?php
/**
 * @file
 * Defines the administration interface for configuring communication with the
 * Pulizon web-service.
 */

/**
 * Administration form for the configureation of the communication with
 * publizon.
 *
 * @return array $form
 */
function publizon_admin_form(&$form_state) {
  // Add some style to the form.
  drupal_add_css(drupal_get_path('module', 'publizon') . '/css/publizon.admin.css');

  // Ensure that the form is stored as an array.
  $form = array(
    '#tree' => TRUE,
  );

  // Load default configuration values.
  $default = variable_get('publizon', array());

  $form['publizon']['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default settings'),
    '#description'=> t('These settings are used to communicate with the publizon web-service, when no user is logged in. They are required to be able to fetch information about products (eg. covers and basic information from Publizon)'),
    '#collapsible' => TRUE,
    '#collapsed' => isset($default['settings']['base_url']) ? TRUE : FALSE,
  );

  $form['publizon']['settings']['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Base URL of the web-service'),
    '#description' => t('The URL of the web-service at Publizon (eg. https://libraryservices.pubhub.dk/)'),
    '#default_value' => isset($default['settings']['base_url']) ?  $default['settings']['base_url'] : 'https://libraryservices.pubhub.dk/',
    '#required' => TRUE,
  );

  $form['publizon']['settings']['retailer_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Retailer ID'),
    '#description' => t("This is the library's identification at publizon, which you should recieve from them."),
    '#default_value' => isset($default['settings']['retailer_id']) ?  $default['settings']['retailer_id'] : '',
    '#required' => TRUE,
  );

  $form['publizon']['settings']['retailer_key_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Retailer key code'),
    '#description' => t('This is a MD5 hash of the code provied with the retailer ID above.'),
    '#default_value' => isset($default['settings']['retailer_key_code']) ?  $default['settings']['retailer_key_code'] : '',
    '#required' => TRUE,
  );

  $form['publizon']['settings']['langcode'] = array(
    '#type' => 'textfield',
    '#title' => t('Language code'),
    '#description' => t('You should also have a language code from Publizon (default: da).'),
    '#default_value' => isset($default['settings']['langcode']) ?  $default['settings']['langcode'] : 'da',
    '#required' => TRUE,
  );

  $form['publizon']['settings']['logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable logging'),
    '#default_value' => isset($default['settings']['logging']) ?  $default['settings']['logging'] : 0,
  );


  // Add libraries fieldset.
  $form['publizon']['libraries'] = array(
    '#type' => 'fieldset',
    '#title' => 'Library configuration',
    '#description' => 'When users login they are mapped to a given library, which needs to have there own Publizon settings to sent to the web-service.',
    '#prefix' => '<div id="libraries-fieldset-wrapper">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $num_libraries = 1;
  if (isset($form_state['values']['publizon']['num_libraries'])) {
    $num_libraries = $form_state['values']['publizon']['num_libraries'];
    if ($form_state['clicked_button']['#value'] == t('Add one more')) {
      $num_libraries++;
    }
    if ($form_state['clicked_button']['#value'] == t('Remove one')) {
      $num_libraries--;
    }
  }


  $form['publizon']['num_libraries'] = array(
    '#type' => 'value',
    '#value' => $num_libraries,
  );

  // Add form elements.
  for ($i = 0; $i < $num_libraries; $i++) {
    $form['publizon']['libraries'][$i] = array(
      '#type' => 'publizon_library_field',
      '#title' => 'Test library field',
      '#description' => 'Test description',
      '#default_value' => array(
        'retailer_id' => $default['libraries'][$i]['retailer_id'],
        'retailer_key_code' => $default['libraries'][$i]['retailer_key_code'],
        'library_name' => $default['libraries'][$i]['library_name'],
        'max_loans' => $default['libraries'][$i]['max_loans']),
    );
  }

  // Add one more button.
  $form['publizon']['libraries']['add_library'] = array(
    '#type' => 'submit',
    '#value' => t('Add one more'),
    '#ahah' => array(
      'event' => 'click',
      'path' => 'admin/settings/publizon/library/ahah',
      'wrapper' => 'libraries-fieldset-wrapper',
      'method' => 'replace',
    ),
    '#attributes' => array('class' => 'add-btn'),
  );

  // Add remove button.
  $form['publizon']['libraries']['remove_library'] = array(
    '#type' => 'submit',
    '#value' => t('Remove one'),
    '#ahah' => array(
      'event' => 'click',
      'path' => 'admin/settings/publizon/library/ahah',
      'wrapper' => 'libraries-fieldset-wrapper',
      'method' => 'replace',
    ),
    '#attributes' => (($num_libraries == 1) ? array('class' => 'remove-btn-hide') : array()),
  );

  return system_settings_form($form);
}

function publizon_admin_form_ahah() {
  // We're starting in step #3, preparing for #4.
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  // Step #4.
  $form = form_get_cache($form_build_id, $form_state);

  // Preparing for #5.
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;

  // Step #5.
  drupal_process_form($form_id, $form, $form_state);

  // Step #6 and #7 and #8.
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  form_set_cache($form_build_id, $form, $form_state);

  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
  );

  $form = form_builder('node_form', $form, $form_state);

  // Step #9.
  $subform = $form['publizon']['libraries'];
  $output = drupal_render($subform);

  // Final rendering callback.
  drupal_json(array('status' => TRUE, 'data' => $output));
}

function publizon_admin_form_validate($form, &$form_state) {
  /**
   * @TODO validate that the information enter is correct.
   */
}