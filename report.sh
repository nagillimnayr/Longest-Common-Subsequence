#!/bin/bash

set -e

python record_data.py

python make_graphs.py
