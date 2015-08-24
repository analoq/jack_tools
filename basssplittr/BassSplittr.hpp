#pragma once
#include <stdexcept>
#include <set>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <iostream>
using namespace std;

class BassSplittr
{
private:
  jack_client_t *client;
  jack_port_t *midi_in, *midi_out;

  struct OpenNote
  {
    uint8_t note;
    uint8_t velocity;

    OpenNote(uint8_t n, uint8_t v)
      : note{n}, velocity{v}
    {
    }

    bool operator<(const OpenNote &other) const
    {
      return note < other.note;
    }
  };

  set<OpenNote> open_notes;
public:
  BassSplittr(const char client_name[])
  {
    client = jack_client_open(client_name, static_cast<jack_options_t>(0), NULL);
	  if (client == 0)
		  throw runtime_error{"jack server not running?"};

	  midi_in = jack_port_register (client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    midi_out = jack_port_register(client, "bass", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
  }

  void start()
  {
	  jack_set_process_callback (client, process, this);
	  if ( jack_activate(client) )
		  throw runtime_error{"cannot activate client"};
  }

  ~BassSplittr()
  {
  	jack_client_close(client);
  }

  int getLowest()
  {
    if ( !open_notes.empty() )
    {
      auto it = open_notes.begin();
      return (*it).note;
    }
    else
      return -1;
  }

  const OpenNote &getLowestNote()
  {
    auto it = open_notes.begin();
    return *it;
  }

  static int process(jack_nframes_t frames, void *arg)
  {
    BassSplittr *bass_splittr = static_cast<BassSplittr *>(arg);
	  // handle midi
	  void *port_buffer_in = jack_port_get_buffer(bass_splittr->midi_in, frames);
	  void *port_buffer_out = jack_port_get_buffer(bass_splittr->midi_out, frames);
    jack_midi_clear_buffer(port_buffer_out);

	  jack_midi_event_t event;
    for ( int event_index = 0;
          !jack_midi_event_get (&event, port_buffer_in, event_index);
          event_index ++ )
	  {
      int last_lowest = bass_splittr->getLowest();
      switch ( event.buffer[0] & 0xF0 )
      {
        case 0x90:
          if ( event.buffer[2] )
          {
            bass_splittr->open_notes.emplace(event.buffer[1], event.buffer[2]);

            int current_lowest = bass_splittr->getLowest();
            if ( current_lowest != last_lowest )
            {
              if ( last_lowest != -1 )
              {
                cerr << "off: " << last_lowest << endl;
                // release old bass note
                uint8_t buffer[3] = {static_cast<uint8_t>(0x8F & event.buffer[0]),
                                     static_cast<uint8_t>(last_lowest),
                                     0};
                jack_midi_event_write(port_buffer_out, event.time, buffer, 3);
              }

              // new bass note
              cerr << "on: " << current_lowest << endl;
              jack_midi_event_write(port_buffer_out, event.time, event.buffer, event.size);
            }
            break;
          }
        case 0x80:
          bass_splittr->open_notes.erase(OpenNote {event.buffer[1], event.buffer[2]});

          if ( last_lowest == event.buffer[1] )
          {
            cerr << "off: " << last_lowest << endl;
            // release old bass note
            jack_midi_event_write(port_buffer_out, event.time, event.buffer, event.size);

            int current_lowest = bass_splittr->getLowest();
            if ( current_lowest != -1 )
            {
              // new bass note
              cerr << "on: " << current_lowest << endl;
              uint8_t buffer[3] = {static_cast<uint8_t>(0x90 | (event.buffer[0] & 0x0F)),
                                   static_cast<uint8_t>(current_lowest),
                                   bass_splittr->getLowestNote().velocity};
              jack_midi_event_write(port_buffer_out, event.time, buffer, 3);
            }
          }
          break;
      }
	  }

	  return 0;
  }
};

