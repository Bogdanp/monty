# This program defines a Person data type, creates an instance of it
# and then prints it.
record Person
  String first_name,
  String last_name,
  Integer age,
end

print(Person("Jim", "Gordon", 32))