var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});
var messageKeys = require('message_keys');

var xhrRequest = function(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getLastfmTracks(claySettings) {
  var apiKey = claySettings.LastfmAPIKey;
  var username = claySettings.LastfmUsername;
  var showUsername = claySettings.ShowUsername;
  var showTimestamp = claySettings.ShowTimestamp;
  var showTrackTotal = claySettings.ShowTrackTotal;

  var limit = 3;
  var url = 'http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks'
    + '&user=' + username + '&api_key=' + apiKey
    + '&limit=' + limit
    + '&format=json';

  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);

      var dict = {};
      // 0 : track name
      // 1 : artist
      // 2 : timestamp

      if (json.recenttracks) {
        var tracks = json.recenttracks.track;
        json.recenttracks.track.forEach(function(track, i) {
          dict[i + '00'] = track.name;
          dict[i + '01'] = track.artist['#text'];
          dict[i + '02'] = track.date ? track.date['#text'] : 'Now Playing';
        });

        dict[messageKeys.LastfmUsername] = username;
        dict[messageKeys.ShowUsername] = showUsername;
        dict[messageKeys.ShowTimestamp] = showTimestamp;
        dict[messageKeys.ShowTrackTotal] = showTrackTotal;

        Pebble.sendAppMessage(dict,
          function(success) {
            console.log('Last.fm track info sent to Pebble successfully!');
          },
          function(error) {
            console.log('Error sending Last.fm track info to Pebble!');
            console.log(JSON.stringify(error));
          }
        );
      }
    }
  );
}

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) {
    return;
  }

  var dict = clay.getSettings(e.response);
  if (dict) {
    var claySettings = {
      LastfmAPIKey: dict[messageKeys.LastfmAPIKey],
      LastfmUsername: dict[messageKeys.LastfmUsername],
      ShowUsername: dict[messageKeys.ShowUsername],
      ShowTimestamp: dict[messageKeys.ShowTimestamp],
      ShowTrackTotal: dict[messageKeys.ShowTrackTotal]
    };
    getLastfmTracks(claySettings);
  }
});

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  var claySettings = JSON.parse(localStorage.getItem('clay-settings'));
  if (claySettings) {
    getLastfmTracks(claySettings);
  }
});
