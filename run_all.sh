#!/bin/zsh
trap "exit" INT
set -e

# Run originals
echo ">> Gecode Original"
./run_original_gecode.sh gbac
./run_original_gecode.sh rcpsp-wet
./run_original_gecode.sh steelmillslab

echo ">> Chuffed Original"
./run_original_chuffed.sh gbac
./run_original_chuffed.sh rcpsp-wet
./run_original_chuffed.sh steelmillslab

# Run Half Reified implementations
echo ">> Gecode on_restart"
./run_restart_gecode.sh gbac
./run_restart_gecode.sh rcpsp-wet
./run_restart_gecode.sh steelmillslab

echo ">> Chuffed on_restarts"
./run_restart_chuffed.sh gbac
./run_restart_chuffed.sh rcpsp-wet
./run_restart_chuffed.sh steelmillslab

# Record Gecode Neighbourhoods
echo ">> Gecode on_restart_record"
./run_record_gecode.sh gbac
./run_record_gecode.sh rcpsp-wet
./run_record_gecode.sh steelmillslab

# Replay Gecode Neighbourhoods
echo ">> Gecode on_restart_replay"
./run_replay_gecode.sh gbac
./run_replay_gecode.sh rcpsp-wet
./run_replay_gecode.sh steelmillslab
