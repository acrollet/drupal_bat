<?php

/**
 * @file
 * Unit editing UI.
 *
 * We make very little use of the EntityAPI interface for this - preferring
 * instead to use views. That offers more flexibility to change a UI that will,
 * more often than not, be end-user facing.
 */

/**
 * UI controller.
 */
class BatUnitUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults. Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {
    $items = array();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%' . $this->entityType;

    $items[$this->path] = array(
      'title' => 'Bookable Units',
      'description' => 'Add, edit, and update bookable units.',
      'page callback' => 'system_admin_menu_block_page',
      'access arguments' => array('access bookable units overview'),
      'file path' => drupal_get_path('module', 'system'),
      'file' => 'system.admin.inc',
      'weight' => 10,
    );

    // Change the add page menu to multiple types of entities.
    $items[$this->path . '/add'] = array(
      'title' => 'Add a Bookable Unit',
      'description' => 'Create a new bookable unit.',
      'page callback'  => 'bat_unit_add_page',
      'access callback'  => '_bat_unit_add_access',
      'type' => MENU_NORMAL_ITEM,
      'weight' => 20,
      'file' => 'bat_unit.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    // Add menu items to add each different type of units.
    foreach (bat_unit_get_types() as $type) {
      $items[$this->path . '/add/' . $type->type] = array(
        'title' => 'Add @unit_type_label unit',
        'title arguments' => array('@unit_type_label' => $type->label),
        'page callback' => 'bat_unit_create_form_wrapper',
        'page arguments' => array($type->type),
        'access callback' => 'bat_unit_access',
        'access arguments' => array('create', bat_unit_create(array('type' => $type->type, 'uid' => 0))),
        'file' => 'bat_unit.admin.inc',
        'file path' => drupal_get_path('module', $this->entityInfo['module']),
      );
    }

    // Loading and editing Unit entities.
    $items[$this->path . '/unit/' . $wildcard] = array(
      'title callback' => 'bat_unit_page_title',
      'title arguments' => array($id_count + 1),
      'page callback' => 'bat_unit_page_view',
      'page arguments' => array($id_count + 1),
      'access callback' => 'bat_unit_access',
      'access arguments' => array('view', $id_count + 1),
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    $items[$this->path . '/unit/' . $wildcard . '/view'] = array(
      'title' => 'View',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    $items[$this->path . '/unit/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'page callback' => 'bat_unit_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'bat_unit_access',
      'access arguments' => array('update', $id_count + 1),
      'weight' => 0,
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
      'file' => 'bat_unit.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    $items[$this->path . '/unit/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'bat_unit_delete_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'bat_unit_access',
      'access arguments' => array('delete', $id_count + 1),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'bat_unit.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    // Menu item for viewing units.
    $items['unit/' . $wildcard] = array(
      'title callback' => 'bat_unit_page_title',
      'title arguments' => array(1),
      'page callback' => 'bat_unit_page_view',
      'page arguments' => array(1),
      'access callback' => 'bat_unit_access',
      'access arguments' => array('view', 1),
      'type' => MENU_CALLBACK,
    );
    return $items;
  }

  /**
   * Creates the markup for the add Bookable Unit Entities page within the class
   * so it can easily be extended/overridden.
   */
  public function addPage() {
    $item = menu_get_item();
    $content = system_admin_menu_block($item);

    if (count($content) == 1) {
      $item = array_shift($content);
      drupal_goto($item['href']);
    }
    return array(
      '#theme' => 'bat_unit_add_list',
      '#content' => $content,
    );
  }

}

/**
 * Form callback wrapper: edit a Bookable Unit.
 *
 * @param $unit
 *   The BatUnit object being edited by this form.
 *
 * @see bat_unit_edit_form()
 */
function bat_unit_form_wrapper($unit) {
  // Add the breadcrumb for the form's location.
  bat_unit_set_breadcrumb();
  drupal_set_title(t('Edit @unit_name', array('@unit_name' => $unit->name)));
  $unit->date = format_date($unit->created, 'custom', 'Y-m-d H:i:s O');
  $account = user_load($unit->uid);
  $unit->author_name = isset($account->name) ? $account->name : '';
  return drupal_get_form('bat_unit_edit_form', $unit);
}

/**
 * Form callback wrapper: create a Unit.
 *
 * @param $type
 *   The Unit type for the unit to be created.
 */
function bat_unit_create_form_wrapper($type) {
  global $user;
  // Add the breadcrumb for the form's location.
  bat_unit_set_breadcrumb();
  $unit = bat_unit_create(array('type' => $type, 'uid' => $user->uid));
  $unit->created = REQUEST_TIME;
  $unit->author_name = $user->name;
  $unit->status = 1;
  return drupal_get_form('bat_unit_edit_form', $unit);
}

/**
 * Form callback wrapper: delete a unit.
 *
 * @param BatUnit $unit
 *   The unit object being edited by this form.
 *
 * @see bat_unit_edit_form()
 */
function bat_unit_delete_form_wrapper($unit) {
  // Add the breadcrumb for the form's location.
  bat_unit_set_breadcrumb();
  return drupal_get_form('bat_unit_delete_form', $unit);
}

/**
 * Form callback: create or edit a unit.
 *
 * @param BatUnit $unit
 *   The BatUnit object to edit or for a create form an empty unit object
 *   with only a unit type defined.
 */
function bat_unit_edit_form($form, &$form_state, $unit) {
  $form['#attributes']['class'][] = 'bat-management-form bat-unit-edit-form';
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'bat_unit') . '/css/bat_unit.css',
    drupal_get_path('module', 'bat_ui') . '/css/bat_ui.css',
  );

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $unit->type,
  );

  // Add the default field elements.
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Unit name'),
    '#default_value' => isset($unit->name) ? $unit->name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -99,
  );

  // Add the field related form elements.
  $form_state['bat_unit'] = $unit;
  field_attach_form('bat_unit', $unit, $form, $form_state, isset($unit->language)? $unit->language : NULL);

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  // Unit availability information for administrators.
  $form['availability'] = array(
    '#type' => 'fieldset',
    '#group' => 'additional_settings',
    '#title' => t('Status'),
    '#tree' => FALSE,
    '#weight' => 80,
    '#attributes' => array(
      'class' => array('unit-form-availability '),
    ),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'bat_unit') . '/js/bat_unit.js'),
    ),
  );

  $form['availability']['bookable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Bookable'),
    '#description' => t('Uncheck this to temporarily prevent the unit from being booked or appearing in any searches without affecting the scheduled availability on the calendar.'),
    '#default_value' => isset($unit->bookable) ? $unit->bookable : 1,
  );

  // Unit author information for administrators.
  $form['author'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('bypass bat_unit entities access'),
    '#title' => t('Authoring information'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('unit-form-author'),
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'bat_unit') . '/js/bat_unit.js',
        array(
          'type' => 'setting',
          'data' => array('anonymous' => variable_get('anonymous', t('Anonymous'))),
        ),
      ),
    ),
    '#weight' => 90,
  );

  $form['author']['author_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Authored by'),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => !empty($unit->author_name) ? $unit->author_name : '',
    '#weight' => -1,
    '#description' => t('Leave blank for %anonymous.', array('%anonymous' => variable_get('anonymous', t('Anonymous')))),
  );

  $form['author']['date'] = array(
    '#type' => 'textfield',
    '#title' => t('Authored on'),
    '#maxlength' => 25,
    '#description' => t('Format: %time. The date format is YYYY-MM-DD and %timezone is the time zone offset from UTC. Leave blank to use the time of form submission.', array('%time' => !empty($unit->date) ? date_format(date_create($unit->date), 'Y-m-d H:i:s O') : format_date($unit->created, 'custom', 'Y-m-d H:i:s O'), '%timezone' => !empty($unit->date) ? date_format(date_create($unit->date), 'O') : format_date($unit->created, 'custom', 'O'))),
    '#default_value' => !empty($unit->date) ? $unit->date : '',
  );

  // Unit publishing options for administrators.
  $form['options'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('bypass bat_unit entities access'),
    '#title' => t('Publishing options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('unit-form-published'),
    ),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'bat_unit') . '/js/bat_unit.js'),
    ),
    '#weight' => 95,
  );

  $form['options']['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => $unit->status,
  );

  // If we are creating a new booking unit, bulk creation info.
  if ($unit->unit_id == '') {
    $form['multi'] = array(
      '#type' => 'fieldset',
      '#group' => 'additional_settings',
      '#title' => t('Create multiple units'),
      '#description' => t('Save time by creating multiple bookable units at once all containing the settings above. You can still personalise each one later on.'),
      '#weight' => 85,
      '#attributes' => array(
        'class' => array('unit-form-multiple'),
      ),
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'bat_unit') . '/js/bat_unit.js'),
      ),
    );

    $form['multi']['multiple'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#default_value' => 1,
      '#title' => t('How many units should we create'),
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#tree' => FALSE,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Unit'),
    '#submit' => $submit + array('bat_unit_edit_form_submit'),
  );

  if (!empty($unit->name) && bat_unit_access('delete', $unit)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete Unit'),
      '#suffix' => l(t('Cancel'), 'admin/bat/units'),
      '#submit' => $submit + array('bat_unit_form_submit_delete'),
      '#weight' => 45,
    );
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'bat_unit_edit_form_validate';
  return $form;
}

/**
 * Form API validate callback for the booking unit form.
 */
function bat_unit_edit_form_validate(&$form, &$form_state) {
  // Notify field widgets to validate their data.
  entity_form_field_validate('bat_unit', $form, $form_state);

  if (!empty($form_state['values']['multiple']) && (!is_numeric($form_state['values']['multiple'])|| $form_state['values']['multiple'] < 1)) {
    form_set_error('Multiple units error', t('%name: you must enter at least 1 as the number of multiple units.', array('%name' => t('Multiple units'))));
  }
}

/**
 * Form API submit callback for the Unit form.
 */
function bat_unit_edit_form_submit(&$form, &$form_state) {
  $units = array();

  if ($form_state['bat_unit']->unit_id == '') {
    $multiple = $form_state['values']['multiple'];

    $name = $form_state['values']['name'];
    for ($i = 1; $i <= $multiple; $i++) {
      $units[$i] = bat_unit_create(array('type' => $form_state['bat_unit']->type));
      $form_state['bat_unit'] = $units[$i];
      if ($i > 1) {
        $form_state['values']['name'] = $name . ' - ' . $i;
      }

      $units[$i] = entity_ui_controller('bat_unit')->entityFormSubmitBuildEntity($form, $form_state);

      // Add in created and changed times.
      $units[$i]->is_new = isset($units[$i]->is_new) ? $units[$i]->is_new : 0;

      $units[$i]->changed = time();
    }
  }
  else {
    $units[0] = $form_state['bat_unit'];
    $units[0] = entity_ui_controller('bat_unit')->entityFormSubmitBuildEntity($form, $form_state);
    $units[0]->changed = time();
  }

  foreach ($units as $unit) {

    if (isset($unit->author_name)) {
      if ($account = user_load_by_name($unit->author_name)) {
        $unit->uid = $account->uid;
      }
      else {
        $unit->uid = 0;
      }
    }

    $unit->created = !empty($unit->date) ? strtotime($unit->date) : REQUEST_TIME;
    $unit->save();
    drupal_set_message(t('Bookable unit @name saved', array('@name' => $unit->name)));
  }

  $form_state['redirect'] = 'admin/bat/units';
}

/**
 * Form API submit callback for the delete button.
 */
function bat_unit_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/bat/units/unit/' . $form_state['bat_unit']->unit_id . '/delete';
}

/**
 * Form callback: confirmation form for deleting a unit.
 *
 * @param $unit
 *   The unit to delete.
 *
 * @see confirm_form()
 */
function bat_unit_delete_form($form, &$form_state, $unit) {
  $form_state['bat_unit'] = $unit;

  $form['#submit'][] = 'bat_unit_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete Unit %name?', array('%name' => $unit->name)),
    'admin/bat/units/unit',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for unit_delete_form
 */
function bat_unit_delete_form_submit($form, &$form_state) {
  $unit = $form_state['bat_unit'];

  bat_unit_delete($unit);

  drupal_set_message(t('The unit %name has been deleted.', array('%name' => $unit->name)));
  watchdog('bat', 'Deleted unit %name.', array('%name' => $unit->name));

  $form_state['redirect'] = 'admin/bat/units';
}

/**
 * Page to add Units.
 */
function bat_unit_add_page() {
  $controller = entity_ui_controller('bat_unit');
  return $controller->addPage();
}

/**
 * Displays the list of available unit types for unit creation.
 *
 * @ingroup themeable
 */
function theme_bat_unit_add_list($variables) {
  $content = $variables['content'];
  $output = '';
  if ($content) {
    $output = '<dl class="unit-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    if (user_access('administer unit types')) {
      $output = '<p>' . t('Bookable Units cannot be added because you have not created any unit types yet. Go to the <a href="@create-unit-type">unit type creation page</a> to add a new unit type.', array('@create-unit-type' => url('admin/bat/units/unit-types/add'))) . '</p>';
    }
    else {
      $output = '<p>' . t('No bookable unit types have been created yet for you to use.') . '</p>';
    }
  }

  return $output;
}

/**
 * Sets the breadcrumb for administrative BAT pages.
 */
function bat_unit_set_breadcrumb() {
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('bat'), 'admin/bat'),
    l(t('bat'), 'admin/bat/units'),
  );

  drupal_set_breadcrumb($breadcrumb);
}
