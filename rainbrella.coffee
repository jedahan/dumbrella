app = require('express').createServer()
raining = -1

app.get '/rain', (req, res) ->
  console.log "sending #{raining} to #{req.headers['user-agent']}"
  res.send "#{raining}"

app.post '/rain', (req, res) ->
  raining = req.param 'rain', 1
  console.log "setting rain to #{raining} from #{req.headers['user-agent']}"
  res.send "#{raining}"

# 70% chance of the weather changing every 10 seconds
setTimeout ->
  raining *= -1 if Math.random() < 0.7
, 10000

app.listen 3000