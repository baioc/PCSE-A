#!/bin/sh
cmdL='make Space run'
cmdR='gdb kernel.bin'

session="work"

# set up tmux
# tmux start-server

# create a new tmux session
tmux new-session -d -s $session

# Select pane 1, set dir to api, run vim
tmux selectp -t 1
tmux send-keys make Space run  C-m

# Split pane 1 horizontal by 65%, start redis-server
tmux splitw -h -p 40

# Select pane 2
tmux selectp -t 2
tmux send-keys make Space run_gdb  C-m

# create a new window called scratch
# tmux new-window -t $session:1 -n scratch

# return to main vim window
# tmux select-window -t $session:0

# Finished setup, attach to the tmux session!
tmux attach-session -t $session
