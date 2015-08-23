#pragma once
#include <stdexcept>
#include <array>
#include <jack/jack.h>
#include <jack/midiport.h>

using namespace std;

class BassTransposr
{
private:
  jack_client_t *client;
  jack_port_t *midi_in, *control_in, *midi_out;
  uint8_t reference_note;
  int8_t offset;
  array<uint8_t, 128> open_notes;

public:
  BassTransposr(const char client_name[], uint8_t r)
    : reference_note{r}, offset{0}
  {
    client = jack_client_open(client_name, static_cast<jack_options_t>(0), NULL);
	  if (client == 0)
		  throw runtime_error{"jack server not running?"};

	  midi_in = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	  control_in = jack_port_register(client, "control in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    midi_out = jack_port_register(client, "out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
  }

  void start()
  {
	  jack_set_process_callback (client, process, this);
	  if ( jack_activate(client) )
		  throw runtime_error{"cannot activate client"};
  }

  ~BassTransposr()
  {
  	jack_client_close(client);
  }

  static int process(jack_nframes_t frames, void *arg)
  {
    BassTransposr *bass_transposr = static_cast<BassTransposr *>(arg);
	  // handle midi
	  void *port_buffer_in = jack_port_get_buffer(bass_transposr->midi_in, frames);
	  void *port_buffer_control = jack_port_get_buffer(bass_transposr->control_in, frames);
	  void *port_buffer_out = jack_port_get_buffer(bass_transposr->midi_out, frames);
    jack_midi_clear_buffer(port_buffer_out);

	  jack_midi_event_t event;

    // handle control in
    for ( int event_index = 0;
          !jack_midi_event_get(&event, port_buffer_control, event_index);
          event_index ++ )
	  {
      switch ( event.buffer[0] & 0xF0 )
      {
        case 0x90:
          bass_transposr->offset = event.buffer[1] - bass_transposr->reference_note;
          break;
      }          
    }

    // handle midi in
    for ( int event_index = 0;
          !jack_midi_event_get(&event, port_buffer_in, event_index);
          event_index ++ )
	  {
      switch ( event.buffer[0] & 0xF0 )
      {
        case 0x90:
          if ( event.buffer[2] ) // note on
          {
            uint8_t new_pitch = event.buffer[1] + bass_transposr->offset;
            bass_transposr->open_notes[event.buffer[1]] = new_pitch;
            event.buffer[1] = new_pitch;
          }
          else // note off
            event.buffer[1] = bass_transposr->open_notes[event.buffer[1]];
          break;

        case 0x80: // note off
          event.buffer[1] = bass_transposr->open_notes[event.buffer[1]];
          break;
      }
      jack_midi_event_write(port_buffer_out, event.time, event.buffer, event.size);
	  }

	  return 0;
  }
};

