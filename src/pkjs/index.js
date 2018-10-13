var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});
var messageKeys = require('message_keys');

var env = require('./env.json');

var lastfmUsername = '';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function hasSpecialCharacters(str) {
  const r = /[\u3040-\u30ff\u3400-\u4dbf\u4e00-\u9fff\uf900-\ufaff\uff66-\uff9f]/;
  return str.match(r);
}

function getLastfmTracks() {
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
          dictionary[i + '00'] = hasSpecialCharacters(track.name) ? '-' : track.name;
          dictionary[i + '01'] = hasSpecialCharacters(track.artist['#text']) ? '-' : track.artist['#text'];
          dictionary[i + '02'] = track.date ? track.date['#text'] : 'Now Playing';
        });

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

Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');

    getLastfmTracks();
  }
);

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  console.log('Got message: ' + JSON.stringify(dict));
});

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  var dict = clay.getSettings(e.response);

  lastfmUsername = dict[messageKeys.LastfmUsername];
  console.log('lastfmUsername: ', lastfmUsername);

  Pebble.sendAppMessage(dict, function() {
    console.log('Message sent successfully: ' + JSON.stringify(dict));
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(e));
  });
});