import re
import time
from dataclasses import asdict, dataclass
from datetime import date, datetime, timedelta
from decimal import Decimal
from typing import Optional


@dataclass
class FormInput:
    enum_ent_sex: str
    cm_ent_height: int
    kg_ent_weight: Decimal
    # enum_ent_bodyshape: str
    enum_ent_smoker: str
    enum_ent_activityLevel: str
    enum_ent_chronicMedication: str
    bool_ent_bpMedication: str
    email: Optional[str] = None
    ResultUrl: Optional[str] = None
    date_ent_dob: Optional[str] = None
    yr_ent_age: Optional[int] = None
    

    def __post_init__(self):
        # Form validations

        # check dateofBirth is valid
        if self.date_ent_dob is None and self.yr_ent_age is None:
            raise ValueError(f"Missing 'date_ent_dob' and 'yr_ent_age'. At least one is needed.")
        elif self.date_ent_dob is None:
            calc_dob = datetime.now() - timedelta(days = self.yr_ent_age * 365.25)
            self.date_ent_dob = f"{calc_dob.year}-{calc_dob.month}-{calc_dob.day}"
        try:
            dob = datetime.strptime(self.date_ent_dob, "%Y-%m-%d").date()
            self.date_ent_dob_epoch = int(
                time.mktime(dob.timetuple())
            )  # Convert to epoch
        except ValueError:
            raise ValueError(f"Invalid date of birth: {self.date_ent_dob}")
        
        if self.yr_ent_age is None:
            dob = datetime.fromtimestamp(self.date_ent_dob_epoch)  # Convert from epoch
            self.yr_ent_age = date.today().year - dob.year

        # Validate email, if it exists
        if self.email is not None and not re.match(r"[^@]+@[^@]+\.[^@]+", self.email):
            raise ValueError(f"Invalid email: {self.email}")

        # Validate age range
        age = date.today().year - dob.year
        if not (18 <= age <= 120):
            raise ValueError(f"Invalid age: {age}")

        # Validate biological sex
        if self.enum_ent_sex not in ["male", "female"]:
            raise ValueError(f"Invalid biological sex: {self.enum_ent_sex}")

        # Coercion
        self.cm_ent_height = int(self.cm_ent_height)
        self.kg_ent_weight = Decimal(round(float(self.kg_ent_weight), 1))

    @property
    def yr_raw_age(self) -> int:
        dob = datetime.fromtimestamp(self.date_ent_dob_epoch)  # Convert from epoch
        return date.today().year - dob.year

    def model_dump(self):
        d = asdict(self)
        for name in dir(self):
            if name not in d and isinstance(
                getattr(self.__class__, name, None), property
            ):
                d[name] = getattr(self, name)
        return d
