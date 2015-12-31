<?php

/**
 * @file
 * Class AbstractEventStyle
 */

namespace Roomify\bat_event;

use Roomify\bat\Event;
use Roomify\bat_event\EventStyle;

abstract class AbstractEventStyle implements EventStyle {

  /**
   *
   */
  public $event;

  /**
   *
   */
  public function __construct(Event $event) {
    $this->event = $event;
  }

}
