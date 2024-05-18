"""
The building blocks for adding a slider menu
With time I'd add it to the main GUI
"""
from tkinter import Tk, Scale, Button, Label
from datetime import datetime

# List of slider values
slider_values = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]

# List of datetime objects
date_times = [
    datetime(2024, 4, 1),
    datetime(2024, 4, 2),
    datetime(2024, 4, 3),
    datetime(2024, 4, 4),
    datetime(2024, 4, 5),
    datetime(2024, 4, 6),
    datetime(2020, 1, 7),
    datetime(2022, 4, 8),
    datetime(2024, 8, 9),
    datetime(2023, 5, 10)
]
# sorted dates
date_times = sorted(date_times)


def update_label():
    min_value = slider_values[min_slider.get()]
    max_value = slider_values[max_slider.get()]
    min_date = date_times[min_slider.get()].strftime("%Y-%m-%d")
    max_date = date_times[max_slider.get()].strftime("%Y-%m-%d")
    label.config(text=f"Min Value: {min_date} | Max Value: {max_date}")

# Function to handle button click
def submit_range():
    update_label()

# Create Tkinter window
window = Tk()
window.title("Choose date range")

# Create Scale widgets for minimum and maximum values
min_slider = Scale(window, from_=0, to=len(slider_values)-1, orient="horizontal", length=300)
min_slider.pack()

max_slider = Scale(window, from_=0, to=len(slider_values)-1, orient="horizontal", length=300)
max_slider.pack()

# Create button to submit the range
submit_button = Button(window, text="Submit Range", command=submit_range)
submit_button.pack()

# Create label to display selected range
label = Label(window, text="Min Value: 0 | Max Value: 0")
label.pack()

# Run the Tkinter event loop
window.mainloop()
