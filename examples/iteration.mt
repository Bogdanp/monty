protocol Iterable
  Iterator iter(self)
end

protocol Iterator[T]
  Boolean has_more(self)
  T get_next(self)
end

record Range
  Integer start,
  Integer end,
  Integer step,
end

extend Range
  def Iterator[Integer] iter(self)
    RangeIterator(self, self.start)
  end
end

record RangeIterator
  Range range,
  Integer current,
end

extend RangeIterator
  def has_more(self)
    self.current + self.step < self.range.end
  end

  def get_next(self)
    # assignment expressions return the previous value of the thing
    # being assigned
    self.current = self.current + self.step
  end
end

def range(end)
  Range(0, end, 1)
end

def range(start, end)
  Range(start, end, 1)
end

def range(start, end, step)
  Range(start, end, step)
end

for i in range(20)
  print(i)
end