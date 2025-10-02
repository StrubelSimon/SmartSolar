import express from "express";
import { batteryRoute } from "./battery";
import { loadRoute } from "./load";
import { pvRoute } from "./pv";

const router = express.Router()

router.use('/', batteryRoute)
router.use('/', loadRoute)
router.use('/', pvRoute)

export { router as mppt }