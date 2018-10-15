var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});
var messageKeys = require('message_keys');

var env = require('./env.json');

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getLastfmTracks(lastfmUsername) {
  var username = lastfmUsername !== '' ? lastfmUsername : env.lastfm.username;
  var apiKey = env.lastfm.apiKey;
  var limit = 3;
  var url = 'http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks'
    + '&user=' + username + '&api_key=' + apiKey
    + '&limit=' + limit
    + '&format=json';

  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);

      var dictionary = {};
      // 0 : track name
      // 1 : artist
      // 2 : timestamp

      if (json.recenttracks) {
        var tracks = json.recenttracks.track;
        json.recenttracks.track.forEach(function(track, i) {
          dictionary[i + '00'] = track.name;
          dictionary[i + '01'] = track.artist['#text'];
          dictionary[i + '02'] = track.date ? track.date['#text'] : 'Now Playing';
        });

        dictionary[messageKeys.LastfmUsername] = lastfmUsername;

        console.log('dictionary', JSON.stringify(dictionary));

        Pebble.sendAppMessage(dictionary,
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
  if (dict && dict[messageKeys.LastfmUsername]) {
    getLastfmTracks(dict[messageKeys.LastfmUsername]);
  }
});

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  var claySettings = JSON.parse(localStorage.getItem('clay-settings'));
  if (claySettings && claySettings.LastfmUsername) {
    getLastfmTracks(claySettings.LastfmUsername);
  }
});
