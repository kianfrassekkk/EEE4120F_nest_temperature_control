from pathlib import Path
from Functions import *
from tkinter import Tk, Canvas, Entry, Text, Button, PhotoImage, BOTH, END
import numpy as np
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import pandas as pd

OUTPUT_PATH = Path(__file__).parent
ASSETS_PATH = OUTPUT_PATH / 'assets'

# variables in this file
hum_int: str = "hum_int"
temp_int: str = "temp_int"
temp_max: str = "temp_max"
temp_min: str = "temp_min"
variables = {hum_int: 10, temp_int: 10, temp_max: 40, temp_min: 25}
data_Arrays = []


# functions in this file
def relative_to_assets(path: str) -> Path:
    return ASSETS_PATH / Path(path)


def reset_vals():
    h_int.delete(0, END)
    t_int.delete(0, END)
    t_max.delete(0, END)
    t_min.delete(0, END)

    h_int.insert(0, '10')
    t_int.insert(0, '10')
    t_max.insert(0, '40')
    t_min.insert(0, '25')


def on_enter(event, widget_name, var):
    try:
        value = float(widget_name.get())
        variables[var] = value
        print(f"User entered {variables[var]} in {var}")
    except ValueError:
        print(f"Invalid input. Please enter a valid number")
        show_message("Value error", "Please insert a valid number")


def on_open_button_click():
    global data_Arrays
    data_Arrays = data_import()


def on_plot_button_click():
    global data_Arrays

    if not data_Arrays:
        print("No data available.")
        return

    # Assuming data_Arrays has at least two columns (Date and Values)
    data = pd.DataFrame(data_Arrays)
    data = data.transpose()

    fig = Figure(figsize=(5, 5), facecolor="#FFFFFF")
    ax1 = fig.add_subplot(111)  # You need to specify the subplot index

    cols = ['tab:blue', 'orange', 'green', 'red']
    for i in range(1, data.shape[1] - 1):
        if i <= 2:
            ax1.plot(data[0], data[i], label=f"Temperature {i}", color=cols[i-1],marker='o')  # Display data points for temperature
            ax1.set_ylabel('Temperature (°C)', color='tab:blue')  # Set y-axis label for temperature data
            ax1.tick_params(axis='y', labelcolor='tab:blue')  # Set y-axis color for temperature data
        else:
            ax2 = ax1.twinx()  # Create a secondary y-axis for humidity data
            ax2.plot(data[0], data[i], label=f"Humidity {i-2}", color=cols[i-1], marker='s')  # Display data points for humidity
            ax2.set_ylabel('Humidity (%)', color='orange')  # Set y-axis label for humidity data
            ax2.tick_params(axis='y', labelcolor='orange')  # Set y-axis color for humidity data

    fig.legend()
    # Create a Tkinter window
    window = Tk()

    # Set the window geometry
    window.geometry("596x391+389+146")  # width x height + x_offset + y_offset

    # Create a canvas
    canvas = FigureCanvasTkAgg(fig, master=window)
    canvas.draw()
    canvas.get_tk_widget().pack(fill=BOTH, expand=True)

    window.mainloop()



window = Tk()

window.geometry("1000x550")
window.configure(bg="#FFFFFF")

canvas = Canvas(
    window,
    bg="#FFFFFF",
    height=550,
    width=1000,
    bd=0,
    highlightthickness=0,
    relief="ridge"
)

canvas.place(x=0, y=0)
image_image_1 = PhotoImage(
    file=relative_to_assets("image_1.png"))
image_1 = canvas.create_image(
    500.0,
    30.0,
    image=image_image_1
)

image_image_2 = PhotoImage(
    file=relative_to_assets("image_2.png"))
image_2 = canvas.create_image(
    687.0,
    341.0,
    image=image_image_2
)

image_image_3 = PhotoImage(
    file=relative_to_assets("image_3.png"))
image_3 = canvas.create_image(
    172.0,
    319.0,
    image=image_image_3
)

image_image_4 = PhotoImage(
    file=relative_to_assets("image_4.png"))
image_4 = canvas.create_image(
    172.0,
    193.0,
    image=image_image_4
)

entry_image_1 = PhotoImage(file=relative_to_assets("t_max.png"))
entry_bg_1 = canvas.create_image(99.5, 338.0, image=entry_image_1)
t_max = Entry(
    bd=0,
    bg="#0091D5",
    fg="#000000",
    highlightthickness=0
)

t_max.insert(0, 40)
t_max.bind('<Return>', lambda event: on_enter(event, t_max, temp_max))
t_max.place(
    x=60.0,
    y=328.0,
    width=79.0,
    height=18.0
)

entry_image_2 = PhotoImage(
    file=relative_to_assets("h_int.png"))
entry_bg_2 = canvas.create_image(
    98.5,
    212.0,
    image=entry_image_2
)
h_int = Entry(
    bd=0,
    bg="#0091D5",
    fg="#000000",
    highlightthickness=0
)

h_int.insert(0, 10)
h_int.bind('<Return>', lambda event: on_enter(event, h_int, hum_int))
h_int.place(
    x=59.0,
    y=202.0,
    width=79.0,
    height=18.0
)

entry_image_3 = PhotoImage(
    file=relative_to_assets("t_min.png"))
entry_bg_3 = canvas.create_image(
    241.5,
    338.0,
    image=entry_image_3
)
t_min = Entry(
    bd=0,
    bg="#0091D5",
    fg="#000000",
    highlightthickness=0
)

t_min.insert(0, 25)
t_min.bind('<Return>', lambda event: on_enter(event, t_min, temp_min))
t_min.place(
    x=202.0,
    y=328.0,
    width=79.0,
    height=18.0
)

entry_image_4 = PhotoImage(
    file=relative_to_assets("t_int.png"))
entry_bg_4 = canvas.create_image(
    241.5,
    212.0,
    image=entry_image_4
)
t_int = Entry(
    bd=0,
    bg="#0091D5",
    fg="#000000",
    highlightthickness=0
)

t_int.insert(0, 10)
t_int.bind('<Return>', lambda event: on_enter(event, t_int, temp_int))
t_int.place(
    x=202.0,
    y=202.0,
    width=79.0,
    height=18.0
)

button_image_1 = PhotoImage(
    file=relative_to_assets("reset_button.png"))
reset_button = Button(
    image=button_image_1,
    borderwidth=0,
    highlightthickness=0,
    command=lambda: (print("reset_button clicked"), reset(variables), reset_vals()),
    relief="flat"
)
reset_button.place(
    x=39.0,
    y=405.0,
    width=116.0,
    height=36.0
)

button_image_2 = PhotoImage(
    file=relative_to_assets("send_button.png"))
send_button = Button(
    image=button_image_2,
    borderwidth=0,
    highlightthickness=0,
    command=lambda: (print("send_button clicked"), settings_export(variables)),
    relief="flat"
)
send_button.place(
    x=184.0,
    y=405.0,
    width=117.0,
    height=35.0
)

button_image_3 = PhotoImage(
    file=relative_to_assets("plot_button.png"))
plot_button = Button(
    image=button_image_3,
    borderwidth=0,
    highlightthickness=0,
    command=lambda: (print("plot_button clicked"), on_plot_button_click()),
    relief="flat"
)
plot_button.place(
    x=546.0,
    y=102.0,
    width=125.0,
    height=36.0
)

button_image_4 = PhotoImage(
    file=relative_to_assets("export_button.png"))
export_button = Button(
    image=button_image_4,
    borderwidth=0,
    highlightthickness=0,
    command=lambda: (print("export_button clicked"), data_export(data_Arrays)),
    relief="flat"
)
export_button.place(
    x=703.0,
    y=102.0,
    width=125.0,
    height=36.0
)

button_image_5 = PhotoImage(
    file=relative_to_assets("open_button.png"))
open_button = Button(
    image=button_image_5,
    borderwidth=0,
    highlightthickness=0,
    command=lambda: on_open_button_click(),
    relief="flat"
)
open_button.place(
    x=389.0,
    y=102.0,
    width=125.0,
    height=36.0
)
window.resizable(False, False)
window.mainloop()
