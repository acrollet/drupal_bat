<?php

/**
 * @file
 * Unit event editing UI.
 *
 * We make very little use of the EntityAPI interface for this - preferring
 * instead to use views. That offers more flexibility to change a UI that will,
 * more often than not, be end-user facing.
 */

/**
 * UI controller.
 */
class BatEventUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   *
   * Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {

    $items = array();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%' . $this->entityType;

    $items[$this->path] = array(
      'title' => 'Events',
      'description' => 'Add edit and update events.',
      'page callback' => 'system_admin_menu_block_page',
      'access arguments' => array('access administration pages'),
      'file path' => drupal_get_path('module', 'system'),
      'file' => 'system.admin.inc',
      'weight' => 10,
    );

    // Change the add page menu to multiple types of entities.
    $items[$this->path . '/add'] = array(
      'title' => 'Add an Event',
      'description' => 'Add a new Event',
      'page callback'  => 'bat_event_add_page',
      'access callback'  => '_bat_event_add_access',
      'type' => MENU_NORMAL_ITEM,
      'weight' => 20,
      'file' => 'bat_event.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    // Add menu items to add each different type of unit.
    foreach (bat_event_get_types() as $type) {
      $items[$this->path . '/add/' . $type->type] = array(
        'title' => 'Add @event_type',
        'title arguments' => array('@event_type' => $type->label),
        'page callback' => 'bat_event_create_form_wrapper',
        'page arguments' => array($type->type),
        'access callback' => 'bat_event_access',
        'access arguments' => array('create', bat_event_create(array('type' => $type->type, 'uid' => 0))),
        'file' => 'bat_event.admin.inc',
        'file path' => drupal_get_path('module', $this->entityInfo['module']),
      );
    }

    // Loading and editing Unit entities.
    $items[$this->path . '/event/' . $wildcard] = array(
      'page callback' => 'bat_event_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'bat_event_access',
      'access arguments' => array('update', $id_count + 1),
      'weight' => 0,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
      'file' => 'bat_event.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );
    $items[$this->path . '/event/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    $items[$this->path . '/event/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'bat_event_delete_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'bat_event_access',
      'access arguments' => array('delete', $id_count + 1),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'bat_event.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );

    // Menu item for viewing units.
    $items['event/' . $wildcard] = array(
      'title callback' => 'bat_event_page_title',
      'title arguments' => array(1),
      'page callback' => 'bat_event_page_view',
      'page arguments' => array(1),
      'access callback' => 'bat_event_access',
      'access arguments' => array('view', 1),
      'type' => MENU_CALLBACK,
    );
    return $items;
  }

  /**
   * Create the markup for the add Event Entities page within the class
   * so it can easily be extended/overriden.
   */
  public function addPage() {
    $item = menu_get_item();
    $content = system_admin_menu_block($item);

    if (count($content) == 1) {
      $item = array_shift($content);
      drupal_goto($item['href']);
    }

    return array(
      '#theme' => 'bat_event_add_list',
      '#content' => $content,
    );
  }

}

/**
 * Form callback wrapper: edit a Event.
 *
 * @param $event
 *   The  BatEvent object being edited by this form.
 *
 * @see bat_event_edit_form()
 */
function bat_event_form_wrapper($event) {
  // Add the breadcrumb for the form's location.
  bat_event_set_breadcrumb();

  // Set the page title.
  $type = bat_event_get_types($event->type);
  drupal_set_title(t('#@event_id - @event_type', array('@event_type' => $type->label, '@event_id' => $event->event_id)));
  $event->date = format_date($event->created, 'custom', 'Y-m-d H:i:s O');
  $account = user_load($event->uid);
  $event->owner_name = $account->name;

  return drupal_get_form('bat_event_edit_form', $event);
}

/**
 * Form callback wrapper: create a Event.
 *
 * @param $event
 *   The Event object being edited by this form.
 *
 * @see bat_event_edit_form()
 */
function bat_event_create_form_wrapper($type) {
  global $user;

  // Add the breadcrumb for the form's location.
  bat_event_set_breadcrumb();

  // Create a event object.
  $event = bat_event_create(array('type' => $type));
  $event->created = REQUEST_TIME;
  $event->owner_name = $user->name;

  return drupal_get_form('bat_event_edit_form', $event);
}

/**
 * Form callback wrapper: delete a event.
 *
 * @param $event
 *   The event object being edited by this form.
 *
 * @see bat_event_edit_form()
 */
function bat_event_delete_form_wrapper($event) {
  // Add the breadcrumb for the form's location.
  bat_event_set_breadcrumb();

  return drupal_get_form('bat_event_delete_form', $event);
}

/**
 * Form callback: create or edit a event.
 *
 * @param $event
 *   The BatEvent object to edit or for a create form an empty event object
 *     with only a event type defined.
 */
function bat_event_edit_form($form, &$form_state, $event) {
  $form['#attributes']['class'][] = 'bat-management-form bat-event-form';
  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('module', 'bat_event') . '/css/bat_event.css',
      drupal_get_path('module', 'bat_ui') . '/css/bat_ui.css',
    ),
    'js' => array(drupal_get_path('module', 'bat') . '/js/bat_date_popup.js'),
  );

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $event->type,
  );

  $form['data']['#tree'] = TRUE;

  // A fieldset to hold the date range fields.
  $form['bat_date_range'] = array(
    '#type' => 'fieldset',
    '#title' => t('Event dates'),
    '#attributes' => array('class' => array('bat-event-date-range-wrapper')),
    '#weight' => -1,
  );

  $form['bat_date_range'] += bat_date_range_fields();

  // Unset the default for max days away for events since we are on the admin.
  drupal_add_js(array('bat' => array('batEventStartDay' => 0)), 'setting');

  // Set the default values for the dates.
  $form['bat_date_range']['bat_start_date']['#default_value'] = isset($event->start_date) ? $event->start_date : '';
  $form['bat_date_range']['bat_end_date']['#default_value'] = isset($event->end_date) ? $event->end_date : '';

  // Check startdate and enddate to avoid damage from dirty input.
  $startdate = '';
  if (isset($_GET['startdate'])) {
    $startdate = (is_numeric(check_plain($_GET['startdate']))) ? check_plain($_GET['startdate']) : '';
    if ($startdate != '') $form['bat_date_range']['bat_start_date']['#default_value'] = gmdate('Y-m-d', $startdate);
  }
  $enddate = '';
  if (isset($_GET['enddate'])) {
    $enddate = (is_numeric(check_plain($_GET['enddate']))) ? check_plain($_GET['enddate']) : '';
    if ($enddate != '') {
      $form['bat_date_range']['bat_end_date']['#default_value'] = gmdate('Y-m-d', $enddate);
    }
  }

  $unitid = '';
  if (isset($_GET['unitid'])) {
    $unitid = (is_numeric(check_plain($_GET['unitid']))) ? check_plain($_GET['unitid']) : '';

    if (bat_unit_load($unitid) == NULL || $startdate == '' || $enddate == '') {
      $unitid = '';
    }
  }

  // Add the field related form elements.
  $form_state['bat_event'] = $event;
  field_attach_form('bat_event', $event, $form, $form_state);

  // Set an #ajax callback for start/end date elements for edited events.
  if (is_numeric($event->event_id)) {
    $form['bat_date_range']['bat_start_date']['#ajax'] = array(
      'callback' => 'bat_event_date_ajax_callback',
      'event' => 'blur',
    );
    $form['bat_date_range']['bat_end_date']['#ajax'] = array(
      'callback' => 'bat_event_date_ajax_callback',
      'event' => 'blur',
    );
  }

  // Management vertical tabs.
  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );


  // Add the user account and e-mail fields.
  $form['user'] = array(
    '#type' => 'fieldset',
    '#title' => t('User information'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#access' => user_access('bypass bat_event entities access'),
    '#group' => 'additional_settings',
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'bat_event') . '/js/bat_event.js',
        array(
          'type' => 'setting',
          'data' => array('anonymous' => variable_get('anonymous', t('Anonymous'))),
        ),
      ),
    ),
    '#weight' => 30,
  );
  $form['user']['owner_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Owned by'),
    '#description' => t('Leave blank for %anonymous.', array('%anonymous' => variable_get('anonymous', t('Anonymous')))),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => !empty($event->owner_name) ? $event->owner_name : '',
    '#weight' => -1,
  );

  // Add a log checkbox and timestamp field to a history tab.
  $form['event_history'] = array(
    '#type' => 'fieldset',
    '#title' => t('Event history'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#group' => 'additional_settings',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'bat_event') . '/js/bat_event.js'),
    ),
    '#weight' => 40,
  );
  $form['event_history']['date'] = array(
    '#type' => 'textfield',
    '#title' => t('Created on'),
    '#description' => t('Format: %time. The date format is YYYY-MM-DD and %timezone is the time zone offset from UTC. Leave blank to use the time of form submission.', array(
      '%time' => !empty($event->date) ? date_format(date_create($event->date), 'Y-m-d H:i:s O') : format_date($event->created, 'custom', 'Y-m-d H:i:s O'),
      '%timezone' => !empty($event->date) ? date_format(date_create($event->date), 'O') : format_date($event->created, 'custom', 'O'),
    )),
    '#maxlength' => 25,
    '#default_value' => !empty($event->created) ? format_date($event->created, 'custom', 'Y-m-d H:i:s O') : '',
  );
  $form['event_history']['created'] = array(
    '#type' => 'hidden',
    '#value' => !empty($event->created) ? format_date($event->created, 'short') : '',
    '#attributes' => array('id' => 'edit-created'),
  );
  $form['event_history']['changed'] = array(
    '#type' => 'hidden',
    '#value' => !empty($event->changed) ? format_date($event->changed, 'short') : '',
    '#attributes' => array('id' => 'edit-changed'),
  );

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 400,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  // Wrapper to show validation messages where the assigned dates are changed
  // and the price is not re-assigned.
  $form['availability_fieldset']['assigned_dates_msg'] = array(
    '#markup' => '<div id="assigned-dates-msg"></div>',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Event'),
    '#submit' => $submit + array('bat_event_edit_form_submit'),
  );

  if (!isset($event->is_new) && bat_event_access('delete', $event)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete Event'),
      '#submit' => $submit + array('bat_event_form_submit_delete'),
      '#weight' => 45,

    );
  }

  // Depending on whether the form is in a popup or a normal page we need to change
  // the behavior of the cancel button
  if (isset($form_state['ajax']) && $form_state['ajax'] == TRUE) {
    $form['actions']['cancel'] = array(
      '#markup' => l(t('Cancel'), 'admin/bat/events', array('attributes' => array('class' => array('ctools-close-modal')))),
      '#weight' => 50,
    );
  }
  else {
    $form['actions']['cancel'] = array(
      '#markup' => l(t('Cancel'), 'admin/bat/events'),
      '#weight' => 50,
    );
  }

  $event_type = bat_event_type_load($event->type);
  if ($event_type->event_granularity == 'hourly') {
    $form['bat_date_range']['bat_start_date']['#date_format'] = 'd/m/Y H:i';
    $form['bat_date_range']['bat_end_date']['#date_format'] = 'd/m/Y H:i';
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'bat_form_start_end_dates_validate';
  $form['#validate'][] = 'bat_event_edit_form_validate';

  return $form;
}

/**
 * Returns a list of unit types user can edit.
 */
function bat_event_edit_form_get_unit_types() {
  $efq = new EntityFieldQuery();
  $efq->entityCondition('entity_type', 'bat_unit');
  $efq->addTag('bat_availability_access');
  $result = $efq->execute();
  $entities = entity_load('bat_unit', array_keys($result['bat_unit']));
  $types = array();
  $all_types = bat_unit_get_types();
  foreach ($entities as $entity) {
    if (!isset($types[$entity->type])) {
      $current_type = $all_types[$entity->type];
      $types[$entity->type] = $current_type->label;
    }
  }
  return $types;
}

/**
 * Returns a set of available units given a unit type, start and end dates.
 *
 * It makes special provisions for the currently selected unit - to consider
 * that as "available" as well.
 *
 * @param string $unit_type
 *   The unit type.
 * @param BatEvent $current_event
 *   The current event.
 * @param DateTime $start_date
 *   Start date.
 * @param DateTime $end_date
 *   End date.
 *
 * @return array|int
 *   Array containing available units or integer to indicate error code.
 */
function bat_event_edit_form_get_units($unit_type, $current_event, $start_date, $end_date) {

  // Set the unit types,
  $unit_types = array($unit_type);
  $current_event_state = '';

  $bookable_units = array();

  // The current unit for this event should also return as available.
  if (isset($current_event->event_status) && isset($current_event->event_id)) {
    $current_event_state = bat_availability_assign_id($current_event->event_id, $current_event->event_status);
  }

  $valid_states = array_keys(array_filter(variable_get('bat_valid_availability_states', drupal_map_assoc(array(BAT_AVAILABLE, BAT_ON_REQUEST)))));

  if (variable_get('bat_price_calculation', BAT_PER_NIGHT) == BAT_PER_PERSON) {
    $event_parameters = array(array('adults' => 1, 'children' => 0, 'childrens_age' => array()));
  }
  else {
    $event_parameters = array(array('adults' => 1, 'children' => 0));
  }

  /*if (!empty($current_event->event_id)) {
    $ac = new AvailabilityAgent($start_date, $end_date, $event_parameters, 1, array_merge($valid_states, array(bat_availability_assign_id($current_event->event_id, '0'), bat_availability_assign_id($current_event->event_id))), $unit_types);
  }
  else {
    $ac = new AvailabilityAgent($start_date, $end_date, $event_parameters, 1, array_merge($valid_states, array($current_event_state)), $unit_types);
  }
  $available_units = $ac->checkAvailability(TRUE);*/

  if (is_array($available_units)) {
    foreach ($available_units as $type => $units_per_price) {
      foreach ($units_per_price as $price => $units) {
        foreach ($units as $unit_id => $unit) {
          if (bat_unit_access('update availability', $unit['unit'])) {
            $bookable_units[$type][$price][$unit_id] = $unit;
          }
        }
      }
    }
  }

  return empty($bookable_units) ? BAT_NO_MATCH : $bookable_units;
}

/**
 * Form API validate callback for the event form.
 */
function bat_event_edit_form_validate(&$form, &$form_state) {
  // Notify field widgets to validate their data.
  entity_form_field_validate('bat_event', $form, $form_state);
}

/**
 * Form API submit callback for the Event form.
 */
function bat_event_edit_form_submit(&$form, &$form_state) {
  // We also need appropriate named variables for start and end date.
  // It's simpler to do this than change all the other code for now.
  $form_state['values']['start_date'] = $form_state['values']['bat_start_date'];
  $form_state['values']['end_date'] = $form_state['values']['bat_end_date'];

  // If we are dealing with a new event.
  if ($form_state['bat_event']->event_id == '') {

    $event = bat_event_create(array('type' => $form_state['bat_event']->type));
    $form_state['bat_event'] = $event;

    $event = entity_ui_controller('bat_event')->entityFormSubmitBuildEntity($form, $form_state);

    $event->is_new = isset($event->is_new) ? $event->is_new : 0;
  }
  else {
    $event = entity_ui_controller('bat_event')->entityFormSubmitBuildEntity($form, $form_state);
  }

  // Add in created and changed times.
  $event->created = !empty($event->date) ? strtotime($event->date) : REQUEST_TIME;
  $event->changed = time();

  // Add in the event owner.
  if ($account = user_load_by_name($event->owner_name)) {
    $event->uid = $account->uid;
  }
  else {
    $event->uid = 0;
  }

  // Save event.
  $event->save();

  // Add the event to $form_state to be altered by other submit handlers.
  $form_state['event'] = $event;

  $event_unit_reference = field_get_items('bat_event', $event, 'event_unit_reference');
  if ($event_unit_reference !== FALSE) {
    $unit_id = $event_unit_reference[0]['target_id'];
  }

  $form_state['redirect'] = 'admin/bat/events';
}

/**
 * Form API submit callback for the delete button.
 */
function bat_event_form_submit_delete($form, &$form_state) {
  if (isset($form_state['ajax'])) {
    bat_event_delete($form_state['bat_event']);
    drupal_set_message(t('The event has been removed'));
    $form_state['event_deleted'] = TRUE;
  }
  else {
    $form_state['redirect'] = 'admin/bat/events/event/' . $form_state['bat_event']->event_id . '/delete';
  }
}

/**
 * Form callback: confirmation form for deleting a event.
 *
 * @param $event
 *   The event to delete.
 *
 * @see confirm_form()
 */
function bat_event_delete_form($form, &$form_state, $event) {
  $form_state['bat_event'] = $event;

  $form['#submit'][] = 'bat_event_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete Event ?'),
    'admin/bat/events/event',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for event_delete_form.
 */
function bat_event_delete_form_submit($form, &$form_state) {
  $event = $form_state['bat_event'];

  bat_event_delete($event);

  drupal_set_message(t('The event has been deleted.'));
  watchdog('bat', 'Deleted event %event_id.', array('%event_id' => $event->event_id));

  $form_state['redirect'] = 'admin/bat/events';
}

/**
 * Page to add Units.
 *
 * @todo Pass this through a proper theme function
 */
function bat_event_add_page() {
  $controller = entity_ui_controller('bat_event');
  return $controller->addPage();
}

/**
 * Displays the list of available unit types for unit creation.
 *
 * @ingroup themeable
 */
function theme_bat_event_add_list($variables) {
  $content = $variables['content'];

  if ($content) {
    $output = '<dl class="event-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    if (user_access('administer event types')) {
      $output = '<p>' . t('Events cannot be added because you have not created any event types yet. Go to the <a href="@create-event-type">event type creation page</a> to add a new event type.', array('@create-event-type' => url('admin/bat/events/event-types/add'))) . '</p>';
    }
    else {
      $output = '<p>' . t('No event types have been created yet for you to use.') . '</p>';
    }
  }

  return $output;
}

/**
 * Sets the breadcrumb for administrative unit pages.
 */
function bat_event_set_breadcrumb() {
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('BAT'), 'admin/bat'),
    l(t('Events'), 'admin/bat/events'),
  );

  drupal_set_breadcrumb($breadcrumb);
}
