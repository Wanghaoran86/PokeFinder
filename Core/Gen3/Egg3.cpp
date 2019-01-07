/*
 * This file is part of PokéFinder
 * Copyright (C) 2017 by Admiral_Fish, bumba, and EzPzStreamz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Egg3.hpp"

Egg3::Egg3()
{
    maxResults = 100000;
    initialFrame = 1;
    seed = 0;
    tid = 12345;
    sid = 54321;
    psv = tid ^ sid;
}

Egg3::Egg3(u32 maxFrame, u32 initialFrame, u16 tid, u16 sid, Method method, u32 seed)
{
    maxResults = maxFrame;
    this->initialFrame = initialFrame;
    this->seed = seed;
    this->tid = tid;
    this->sid = sid;
    psv = tid ^ sid;
    frameType = method;

    switch (method)
    {
        case Method::EBred:
            iv1 = 0;
            iv2 = 1;
            inh1 = 3;
            inh2 = 4;
            inh3 = 5;
            par1 = 6;
            par2 = 7;
            par3 = 8;
            break;
        case Method::EBredSplit:
            iv1 = 0;
            iv2 = 2;
            inh1 = 4;
            inh2 = 5;
            inh3 = 6;
            par1 = 7;
            par2 = 8;
            par3 = 9;
            break;
        case Method::EBredAlternate:
            iv1 = 0;
            iv2 = 1;
            inh1 = 4;
            inh2 = 5;
            inh3 = 6;
            par1 = 7;
            par2 = 8;
            par3 = 9;
            break;
        case Method::RSBred:
        case Method::FRLGBred:
            iv1 = 2;
            iv2 = 3;
            inh1 = 5;
            inh2 = 6;
            inh3 = 7;
            par1 = 8;
            par2 = 9;
            par3 = 10;
            break;
        case Method::RSBredSplit:
        case Method::FRLGBredSplit:
            iv1 = 1;
            iv2 = 3;
            inh1 = 5;
            inh2 = 6;
            inh3 = 7;
            par1 = 8;
            par2 = 9;
            par3 = 10;
            break;
        case Method::RSBredAlternate:
        case Method::FRLGBredAlternate:
            iv1 = 2;
            iv2 = 3;
            inh1 = 6;
            inh2 = 7;
            inh3 = 8;
            par1 = 9;
            par2 = 10;
            par3 = 11;
            break;
        default:
            break;
    }
}

QVector<Frame3> Egg3::generate(const FrameCompare &compare) const
{
    switch (frameType)
    {
        case Method::EBredPID:
            return generateEmeraldPID(compare);
        case Method::EBred:
        case Method::EBredSplit:
        case Method::EBredAlternate:
            return generateEmeraldIVs(compare);
        case Method::RSBred:
        case Method::RSBredAlternate:
        case Method::FRLGBred:
        case Method::FRLGBredAlternate:
            {
                QVector<Frame3> lower = generateLower(compare);
                return lower.isEmpty() ? lower : generateUpper(lower, compare);
            }
        default:
            return QVector<Frame3>();
    }
}

void Egg3::setParents(const QVector<u8> &parent1, const QVector<u8> &parent2)
{
    this->parent1 = parent1;
    this->parent2 = parent2;
}

void Egg3::setMinRedraw(const u8 &value)
{
    minRedraw = value;
}

void Egg3::setMaxRedraw(const u8 &value)
{
    maxRedraw = value;
}

void Egg3::setCompatability(const int &value)
{
    compatability = value;
}

void Egg3::setCalibration(const u8 &value)
{
    calibration = value;
}

void Egg3::setEverstone(bool value)
{
    everstone = value;
}

void Egg3::setMinPickup(const u32 &value)
{
    minPickup = value;
}

void Egg3::setMaxPickup(const u32 &value)
{
    maxPickup = value;
}

u32 Egg3::getSeed() const
{
    return seed;
}

void Egg3::setSeed(const u32 &value)
{
    seed = value;
}

void Egg3::setPickupSeed(const u16 &value)
{
    pickupSeed = value;
}

QVector<Frame3> Egg3::generateEmeraldPID(const FrameCompare &compare) const
{
    QVector<Frame3> frames;

    u32 i;
    u32 pid = 0;

    PokeRNG rng(seed, initialFrame - 1);
    auto *rngArray = new u16[maxResults + 19];
    for (u32 x = 0; x < maxResults + 19; x++)
    {
        rngArray[x] = rng.nextUShort();
    }

    u32 val = initialFrame;

    u32 max = maxResults - initialFrame + 1;
    for (u32 cnt = 0; cnt < max; cnt++, val++)
    {
        for (u8 redraw = minRedraw; redraw <= maxRedraw; redraw++)
        {
            Frame3 frame(tid, sid, psv);
            frame.setGenderRatio(compare.getGenderRatio());

            if (((rngArray[cnt] * 100) / 0xFFFF) >= compatability)
            {
                continue;
            }

            u16 offset = calibration + 3 * redraw;

            i = 1;

            bool flag = everstone ? (rngArray[cnt + i++] >> 15) == 0 : false;

            PokeRNG trng((val - offset) & 0xFFFF);

            if (!flag)
            {
                // Lower PID
                pid = (rngArray[i + cnt] % 0xFFFE) + 1;

                // Upper PID
                pid |= trng.nextUInt() & 0xFFFF0000;

                frame.setPID(pid, pid >> 16, pid & 0xFFFF);
                frame.setNature(pid % 25);
            }
            else
            {
                do
                {
                    // VBlank at 17 from starting PID generation
                    // Adjusted i value is 19
                    // Skip at this point since spread is unlikely to occur
                    if (i == 19)
                    {
                        break;
                    }

                    // generate lower
                    pid = rngArray[cnt + i++];

                    // generate upper
                    pid |= trng.nextUInt() & 0xFFFF0000;
                }
                while (pid % 25 != everstoneNature);

                if (i == 19)
                {
                    continue;
                }

                frame.setPID(pid, pid >> 16, pid & 0xFFFF);
                frame.setNature(everstoneNature);
            }

            if (compare.comparePID(frame))
            {
                frame.setFrame(cnt + initialFrame - offset);
                frame.setOccidentary(redraw);
                frames.append(frame);
            }
        }
    }

    std::sort(frames.begin(), frames.end(), [](const Frame3 & frame1, const Frame3 & frame2)
    {
        return frame1.getFrame() < frame2.getFrame();
    });

    delete[] rngArray;
    return frames;
}

QVector<Frame3> Egg3::generateEmeraldIVs(const FrameCompare &compare) const
{
    QVector<Frame3> frames;

    PokeRNG rng(seed, initialFrame - 1);
    auto *rngArray = new u16[maxResults + 10];
    for (u32 x = 0; x < maxResults + 10; x++)
    {
        rngArray[x] = rng.nextUShort();
    }

    u32 max = maxResults - initialFrame + 1;
    for (u32 cnt = 0; cnt < max; cnt++)
    {
        Frame3 frame(tid, sid, psv);
        frame.setInheritance(rngArray[iv1 + cnt], rngArray[iv2 + cnt], rngArray[par1 + cnt], rngArray[par2 + cnt], rngArray[par3 + cnt],
                             rngArray[inh1 + cnt], rngArray[inh2 + cnt], rngArray[inh3 + cnt], parent1, parent2, true);

        if (compare.compareIVs(frame))
        {
            frame.setFrame(cnt + initialFrame);
            frames.append(frame);
        }
    }

    delete[] rngArray;
    return frames;
}

QVector<Frame3> Egg3::generateLower(const FrameCompare &compare) const
{
    QVector<Frame3> frames;

    PokeRNG rng(seed, initialFrame - 1);
    auto *rngArray = new u16[maxResults + 2];
    for (u32 x = 0; x < maxResults + 2; x++)
    {
        rngArray[x] = rng.nextUShort();
    }

    u32 max = maxResults - initialFrame + 1;
    for (u32 cnt = 0; cnt < max; cnt++)
    {
        if (((rngArray[cnt] * 100) / 0xFFFF) >= compatability)
        {
            continue;
        }

        Frame3 frame(tid, sid, psv);
        frame.setPID((rngArray[1 + cnt] % 0xFFFE) + 1);

        if (compare.compareGender(frame))
        {
            frame.setFrame(cnt + initialFrame);
            frames.append(frame);
        }
    }

    delete[] rngArray;
    return frames;
}

QVector<Frame3> Egg3::generateUpper(const QVector<Frame3> &lower, const FrameCompare &compare) const
{
    QVector<Frame3> upper;

    PokeRNG rng(pickupSeed, minPickup - 1);
    auto *rngArray = new u16[maxPickup + 12];
    for (u32 x = 0; x < maxPickup + 12; x++)
    {
        rngArray[x] = rng.nextUShort();
    }

    u32 max = maxPickup - minPickup + 1;
    for (u32 cnt = 0; cnt < max; cnt++)
    {
        Frame3 frame(tid, sid, psv);
        frame.setPID(rngArray[cnt]);
        frame.setInheritance(rngArray[iv1 + cnt], rngArray[iv2 + cnt], rngArray[par1 + cnt], rngArray[par2 + cnt], rngArray[par3 + cnt],
                             rngArray[inh1 + cnt], rngArray[inh2 + cnt], rngArray[inh3 + cnt], parent1, parent2);

        if (compare.compareIVs(frame))
        {
            frame.setOccidentary(cnt + minPickup);
            upper.append(frame);
        }
    }

    QVector<Frame3> frames;
    for (const auto &low : lower)
    {
        for (auto up : upper)
        {
            up.setPID(low.getPID(), up.getPID());
            if (compare.comparePID(up))
            {
                up.setFrame(low.getFrame());
                frames.append(up);
            }
        }
    }
    return frames;
}