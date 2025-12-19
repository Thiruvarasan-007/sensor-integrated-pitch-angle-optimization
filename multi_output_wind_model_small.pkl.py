import pandas as pd
import numpy as np
from sklearn.multioutput import MultiOutputRegressor
from sklearn.ensemble import RandomForestRegressor
import joblib

# -------- SAMPLE TRAINING DATA --------
# Replace this with real dataset if you have one
np.random.seed(42)

X = np.random.rand(500, 6) * [25, 360, 40, 1000, 90, 180]
y_pitch = X[:, 0] * 1.5
y_power = X[:, 0] ** 2 * 0.4

y = np.column_stack((y_pitch, y_power))

# -------- MODEL --------
model = MultiOutputRegressor(
    RandomForestRegressor(n_estimators=100, random_state=42)
)

model.fit(X, y)

# -------- SAVE MODEL --------
joblib.dump(model, "multi_output_wind_model_small.pkl")

print("Model trained and saved")
