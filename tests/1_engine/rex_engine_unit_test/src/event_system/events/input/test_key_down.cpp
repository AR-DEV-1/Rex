#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/event_system/events/input/key_down.h"
#include "rex_engine/event_system/event_system.h"

TEST_CASE("TEST - Events - Key Down Event")
{
  s32 num_quit_events_fired = 0;
  rex::KeyCode key_code = rex::KeyCode::Unknown;
  auto subscription = rex::event_system::instance()->subscribe<rex::KeyDown>([&num_quit_events_fired, &key_code](const rex::KeyDown& evt)
    {
      ++num_quit_events_fired;
      REX_CHECK(evt.key() == key_code);
    });

  // Immediate event fire
  key_code = rex::KeyCode::A;
  rex::event_system::instance()->fire_event(rex::KeyDown(key_code));
  REX_CHECK(num_quit_events_fired == 1);

  // Queued event fire
  key_code = rex::KeyCode::B;
  rex::event_system::instance()->enqueue_event(rex::KeyDown(key_code));
  REX_CHECK(num_quit_events_fired == 1);

  rex::event_system::instance()->dispatch_queued_events();
  REX_CHECK(num_quit_events_fired == 2);
}