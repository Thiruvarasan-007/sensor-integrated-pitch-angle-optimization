import os
import json
import pandas as pd
import streamlit as st
import joblib

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_FILE = os.path.join(BASE_DIR, "latest_input.json")
MODEL_FILE = os.path.join(BASE_DIR, "multi_output_wind_model_small.pkl")

FEATURES = [
    "Wind Speed (m/s)",
    "Wind Direction (°)",
    "Temperature (°C)",
    "Altitude (m)",
    "Latitude",
    "Longitude"
]

st.set_page_config(page_title="Wind ML", layout="wide")
st.title("⚡ Real-Time Wind Turbine Prediction")

@st.cache_resource
def load_model():
    return joblib.load(MODEL_FILE)

model = load_model()

if not os.path.exists(DATA_FILE):
    st.warning("Waiting for ESP32 data...")
    st.stop()

with open(DATA_FILE, "r") as f:
    d = json.load(f)

data = pd.DataFrame([[
    d["wind_speed"],
    d["wind_deg"],
    d["temperature"],
    d["altitude"],
    d["latitude"],
    d["longitude"]
]], columns=FEATURES)

st.subheader("Live Input Data")
st.dataframe(data.T.rename(columns={0: "Value"}))

pred = model.predict(data)

col1, col2 = st.columns(2)
col1.metric("Pitch Angle (°)", f"{pred[0][0]:.2f}")
col2.metric("Max Output (kW)", f"{pred[0][1]:.2f}")