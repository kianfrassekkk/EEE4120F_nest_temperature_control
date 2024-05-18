from tkinter import filedialog, messagebox
from tkinter import Button, Toplevel, Scale, IntVar
from datetime import datetime
import csv


def data_import() -> list:
    """
    This function imports data from the txt file
    """
    filename = filedialog.askopenfilename(
        title="Select Text File",
        filetypes=[("Text files", "*.txt")]  # Filter for text files only
    )

    if filename:
        # User selected a file, process it here
        print(f"Selected file: {filename}")
        with open(filename, 'r') as file:
            lines = file.readlines()

        # Initialize arrays
        time = []
        temp1 = []
        humd1 = []
        temp2 = []
        humd2 = []
        reason = []

        # Process each line
        for line in lines:
            parts = line.split(',')
            time.append(parts[1].strip())
            datetime_objects = [datetime.strptime(t, "%B %d %Y %H:%M:%S") for t in time]
            temp1.append(float(parts[3].strip()))
            humd1.append(float(parts[5].strip()))
            temp2.append(float(parts[7].strip()))
            humd2.append(float(parts[9].strip()))
            reason.append(parts[11].strip())
        show_message("Success", "Data has been imported")

    else:
        # User canceled the dialog or no file selected
        print("No file selected.")
        show_message("Error", "No file selected")
    return [datetime_objects, temp1, temp2, humd1, humd2, reason]


def data_export(data_arrays):
    """
    Function for exporting data to a txt file
    """
    file_path = filedialog.asksaveasfilename(defaultextension=".txt", initialfile='data_export.txt',
                                             filetypes=[("Text files", "*.txt")])
    if not file_path:  # If the user cancels the dialog
        return

    with open(file_path, 'w', newline='') as txtfile:  # Open the file with .txt extension
        for row in data_arrays:
            txtfile.write(','.join(map(str, row)) + '\n')  # Write each row as comma-separated values

    print(f"Data exported to {file_path}")


def settings_export(settings: dict) -> None:
    """
    Function for exporting user settings
    """
    file_path = filedialog.asksaveasfilename(defaultextension=".csv", initialfile='settings.csv',
                                             filetypes=[("CSV files", "*.csv")])
    if not file_path:  # If the user cancels the dialog
        return

    with open(file_path, 'w', newline='') as csvfile:
        fieldnames = ['Setting', 'Value']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        writer.writeheader()
        for setting, value in settings.items():
            writer.writerow({'Setting': setting, 'Value': value})


def show_message(title: str, info: str) -> None:
    messagebox.showinfo(title=title, message=info)


def reset(dictionary: dict) -> None:
    reset_array = [10, 10, 40, 25]
    keys = list(dictionary.keys())
    for i, key in enumerate(keys):
        if i < len(reset_array):
            dictionary[key] = reset_array[i]
